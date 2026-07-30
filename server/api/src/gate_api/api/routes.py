import structlog
from fastapi import APIRouter, Request, status
from fastapi.responses import JSONResponse
from pydantic import BaseModel

from gate_api.domain.gate_state import GateState
from gate_api.exceptions import MqttConnectionError, TriggerTimeoutError
from gate_api.mqtt.client import GateMqttClient
from gate_api.mqtt.schemas import AckResult, Availability

logger = structlog.get_logger(__name__)

router = APIRouter()


class HealthResponse(BaseModel):
    status: str
    mqtt_connected: bool


class StateResponse(BaseModel):
    state: GateState
    controller_online: bool
    rssi: int | None = None
    uptime_s: int | None = None
    ts: int | None = None


class TriggerResponse(BaseModel):
    request_id: str
    result: AckResult
    ts: int


def _mqtt_client(request: Request) -> GateMqttClient:
    client: GateMqttClient = request.app.state.mqtt_client
    return client


@router.get("/health", response_model=HealthResponse)
def get_health(request: Request) -> HealthResponse:
    client = _mqtt_client(request)
    return HealthResponse(status="ok", mqtt_connected=client.connected)


@router.get("/state", response_model=StateResponse)
def get_state(request: Request) -> StateResponse:
    client = _mqtt_client(request)
    latest = client.latest_state
    online = client.latest_availability == Availability.ONLINE

    if latest is None:
        # No retained gate/state received yet - honest UNKNOWN, never a guess.
        return StateResponse(state=GateState.UNKNOWN, controller_online=online)

    return StateResponse(
        state=latest.state,
        controller_online=online,
        rssi=latest.rssi,
        uptime_s=latest.uptime_s,
        ts=latest.ts,
    )


@router.post("/trigger", response_model=TriggerResponse)
async def post_trigger(request: Request) -> JSONResponse:
    client = _mqtt_client(request)
    try:
        ack = await client.trigger(source="app")
    except MqttConnectionError:
        logger.warning("trigger_failed_not_connected")
        return JSONResponse(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            content={"detail": "not connected to the gate controller"},
        )
    except TriggerTimeoutError as exc:
        logger.warning("trigger_ack_timeout", request_id=exc.request_id)
        return JSONResponse(
            status_code=status.HTTP_504_GATEWAY_TIMEOUT,
            content={"detail": str(exc), "request_id": exc.request_id},
        )

    # Only ACCEPTED means the gate actually moved; the rest are informational
    # per the contract, not error conditions, so they still return 200.
    response_status = (
        status.HTTP_202_ACCEPTED if ack.result == AckResult.ACCEPTED else status.HTTP_200_OK
    )
    body = TriggerResponse(request_id=ack.request_id, result=ack.result, ts=ack.ts)
    return JSONResponse(status_code=response_status, content=body.model_dump(mode="json"))
