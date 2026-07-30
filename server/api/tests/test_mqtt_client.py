import asyncio
import json

import pytest

from gate_api.domain.gate_state import GateState
from gate_api.exceptions import MqttConnectionError
from gate_api.mqtt.client import GateMqttClient
from gate_api.mqtt.schemas import TOPIC_CMD_ACK, TOPIC_STATE


class TestHandleStateMessage:
    def test_valid_payload_updates_latest_state(self, real_mqtt_client: GateMqttClient) -> None:
        # arrange
        payload = json.dumps(
            {"v": 1, "state": "closed", "rssi": -67, "uptime_s": 84120, "ts": 1753036800}
        )

        # act
        real_mqtt_client._handle_message(TOPIC_STATE, payload)

        # assert
        assert real_mqtt_client.latest_state is not None
        assert real_mqtt_client.latest_state.state == GateState.CLOSED

    def test_wrong_schema_version_is_dropped_not_raised(
        self, real_mqtt_client: GateMqttClient
    ) -> None:
        # arrange - a future schema version this server doesn't understand yet
        payload = json.dumps(
            {"v": 2, "state": "closed", "rssi": -67, "uptime_s": 84120, "ts": 1753036800}
        )

        # act
        real_mqtt_client._handle_message(TOPIC_STATE, payload)

        # assert - rejected rather than best-effort parsed, per the contract
        assert real_mqtt_client.latest_state is None

    def test_malformed_json_is_dropped_not_raised(self, real_mqtt_client: GateMqttClient) -> None:
        # act / assert - must not raise out of the MQTT dispatch loop
        real_mqtt_client._handle_message(TOPIC_STATE, "{not json")
        assert real_mqtt_client.latest_state is None


class TestHandleAckMessage:
    def test_resolves_the_matching_pending_future(self, real_mqtt_client: GateMqttClient) -> None:
        # arrange
        loop = asyncio.get_event_loop()
        future: asyncio.Future = loop.create_future()
        real_mqtt_client._pending_acks["req-1"] = future
        payload = json.dumps(
            {"v": 1, "request_id": "req-1", "result": "accepted", "ts": 1753036801}
        )

        # act
        real_mqtt_client._handle_message(TOPIC_CMD_ACK, payload)

        # assert
        assert future.done()
        assert future.result().request_id == "req-1"

    def test_ack_for_unknown_request_id_is_ignored(self, real_mqtt_client: GateMqttClient) -> None:
        # arrange - no pending future registered for this request_id
        payload = json.dumps({"v": 1, "request_id": "unregistered", "result": "accepted", "ts": 1})

        # act / assert - must not raise
        real_mqtt_client._handle_message(TOPIC_CMD_ACK, payload)


class TestTrigger:
    @pytest.mark.asyncio
    async def test_raises_when_not_connected(self, real_mqtt_client: GateMqttClient) -> None:
        # arrange - real_mqtt_client fixture never connects to a broker

        # act / assert
        with pytest.raises(MqttConnectionError):
            await real_mqtt_client.trigger(source="app")
