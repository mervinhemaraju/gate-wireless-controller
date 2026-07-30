import asyncio
import json
import time
import uuid

import aiomqtt
import structlog
from pydantic import ValidationError

from gate_api.config import Settings
from gate_api.exceptions import MalformedPayloadError, MqttConnectionError, TriggerTimeoutError
from gate_api.mqtt.schemas import (
    SCHEMA_VERSION,
    TOPIC_AVAILABILITY,
    TOPIC_CMD_ACK,
    TOPIC_CMD_TRIGGER,
    TOPIC_STATE,
    Availability,
    GateStatePayload,
    TriggerAck,
    TriggerCommand,
)

logger = structlog.get_logger(__name__)

_INITIAL_BACKOFF_S = 1.0
_MAX_BACKOFF_S = 30.0


class GateMqttClient:
    """Owns the single MQTT connection to the broker: caches the latest
    retained state/availability, and correlates gate/cmd/trigger publishes
    with their gate/cmd/ack by request_id.

    Per .claude/skills/mqtt-contract/SKILL.md: gate/cmd/trigger is never
    retained, and duplicate/rejected ack results are not errors - only
    AckResult.ACCEPTED represents an actual gate movement.
    """

    def __init__(self, settings: Settings) -> None:
        self._settings = settings
        self._latest_state: GateStatePayload | None = None
        self._latest_availability: Availability | None = None
        self._pending_acks: dict[str, asyncio.Future[TriggerAck]] = {}
        self._client: aiomqtt.Client | None = None
        self._stop = asyncio.Event()

    @property
    def latest_state(self) -> GateStatePayload | None:
        return self._latest_state

    @property
    def latest_availability(self) -> Availability | None:
        return self._latest_availability

    @property
    def connected(self) -> bool:
        return self._client is not None

    async def run(self) -> None:
        """Connects with reconnect/backoff and dispatches inbound messages
        until stop() is called. Intended to run as a background task for the
        lifetime of the FastAPI app."""
        backoff_s = _INITIAL_BACKOFF_S
        while not self._stop.is_set():
            try:
                async with aiomqtt.Client(
                    hostname=self._settings.mqtt_broker_host,
                    port=self._settings.mqtt_broker_port,
                    identifier=self._settings.mqtt_api_client_id,
                    username=self._settings.mqtt_api_username,
                    password=self._settings.mqtt_api_password.get_secret_value(),
                ) as client:
                    self._client = client
                    backoff_s = _INITIAL_BACKOFF_S
                    await client.subscribe(TOPIC_STATE, qos=1)
                    await client.subscribe(TOPIC_AVAILABILITY, qos=1)
                    await client.subscribe(TOPIC_CMD_ACK, qos=1)
                    logger.info("mqtt_connected", host=self._settings.mqtt_broker_host)
                    async for message in client.messages:
                        payload = message.payload
                        if not isinstance(payload, bytes | bytearray | str):
                            logger.warning(
                                "mqtt_unexpected_payload_type",
                                topic=str(message.topic),
                                payload_type=type(payload).__name__,
                            )
                            continue
                        self._handle_message(str(message.topic), payload)
            except aiomqtt.MqttError as exc:
                self._client = None
                logger.warning("mqtt_disconnected", error=str(exc), retry_in_s=backoff_s)
                await asyncio.sleep(backoff_s)
                backoff_s = min(backoff_s * 2, _MAX_BACKOFF_S)

    def stop(self) -> None:
        self._stop.set()

    def _handle_message(self, topic: str, payload: bytes | bytearray | str) -> None:
        raw = payload.decode() if isinstance(payload, (bytes, bytearray)) else payload
        try:
            if topic == TOPIC_STATE:
                self._latest_state = self._parse_versioned(raw, TOPIC_STATE, GateStatePayload)
            elif topic == TOPIC_AVAILABILITY:
                self._latest_availability = Availability(raw)
            elif topic == TOPIC_CMD_ACK:
                self._handle_ack(raw)
        except MalformedPayloadError as exc:
            logger.warning("mqtt_malformed_payload", topic=topic, reason=exc.reason)
        except ValueError:
            logger.warning("mqtt_malformed_payload", topic=topic, raw=raw)

    def _parse_versioned[
        T: (GateStatePayload, TriggerAck)
    ](self, raw: str, topic: str, model: type[T]) -> T:
        try:
            data = json.loads(raw)
        except json.JSONDecodeError as exc:
            raise MalformedPayloadError(topic, f"invalid json: {exc}") from exc
        if data.get("v") != SCHEMA_VERSION:
            raise MalformedPayloadError(topic, f"unsupported schema version {data.get('v')!r}")
        try:
            return model.model_validate(data)
        except ValidationError as exc:
            raise MalformedPayloadError(topic, str(exc)) from exc

    def _handle_ack(self, raw: str) -> None:
        ack = self._parse_versioned(raw, TOPIC_CMD_ACK, TriggerAck)
        future = self._pending_acks.get(ack.request_id)
        if future is not None and not future.done():
            future.set_result(ack)

    async def trigger(self, source: str) -> TriggerAck:
        if self._client is None:
            raise MqttConnectionError("not connected to the broker")

        request_id = str(uuid.uuid4())
        command = TriggerCommand(request_id=request_id, source=source, ts=int(time.time()))

        loop = asyncio.get_running_loop()
        future: asyncio.Future[TriggerAck] = loop.create_future()
        self._pending_acks[request_id] = future
        try:
            await self._client.publish(
                TOPIC_CMD_TRIGGER,
                payload=command.model_dump_json(),
                qos=1,
                retain=False,
            )
            try:
                return await asyncio.wait_for(
                    future, timeout=self._settings.gate_trigger_ack_timeout_s
                )
            except TimeoutError as exc:
                raise TriggerTimeoutError(
                    request_id, self._settings.gate_trigger_ack_timeout_s
                ) from exc
        finally:
            self._pending_acks.pop(request_id, None)
