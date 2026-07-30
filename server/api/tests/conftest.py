import asyncio
from collections.abc import AsyncIterator

import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient

from gate_api.api.routes import router
from gate_api.mqtt.client import GateMqttClient
from gate_api.mqtt.schemas import Availability, TriggerAck


class FakeMqttClient:
    """Stands in for GateMqttClient in route tests - no real broker needed,
    per the Python rules' "mock external I/O" guidance. Only implements the
    surface routes.py actually calls."""

    def __init__(self) -> None:
        self.connected = True
        self.latest_state = None
        self.latest_availability: Availability | None = Availability.ONLINE
        self.trigger_result: TriggerAck | Exception | None = None
        self.last_trigger_source: str | None = None

    async def trigger(self, source: str) -> TriggerAck:
        self.last_trigger_source = source
        if isinstance(self.trigger_result, Exception):
            raise self.trigger_result
        assert self.trigger_result is not None, "test must set trigger_result"
        return self.trigger_result


@pytest.fixture
def fake_mqtt_client() -> FakeMqttClient:
    return FakeMqttClient()


@pytest.fixture
def app(fake_mqtt_client: FakeMqttClient) -> FastAPI:
    test_app = FastAPI()
    test_app.include_router(router)
    test_app.state.mqtt_client = fake_mqtt_client
    return test_app


@pytest.fixture
def client(app: FastAPI) -> TestClient:
    return TestClient(app)


@pytest.fixture
async def real_mqtt_client() -> AsyncIterator[GateMqttClient]:
    """A real GateMqttClient with no network connection, for unit-testing its
    parsing/correlation logic without a broker."""
    from gate_api.config import Settings

    settings = Settings(
        mqtt_broker_host="localhost",
        mqtt_api_username="test",
        mqtt_api_password="test",  # noqa: S106 - test fixture, not a real secret
    )
    yield GateMqttClient(settings)
    await asyncio.sleep(0)
