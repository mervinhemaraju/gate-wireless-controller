from fastapi.testclient import TestClient

from gate_api.domain.gate_state import GateState
from gate_api.exceptions import MqttConnectionError, TriggerTimeoutError
from gate_api.mqtt.schemas import AckResult, Availability, GateStatePayload, TriggerAck
from tests.conftest import FakeMqttClient


class TestHealth:
    def test_reports_mqtt_connection_state(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.connected = True

        # act
        response = client.get("/health")

        # assert
        assert response.status_code == 200
        assert response.json() == {"status": "ok", "mqtt_connected": True}


class TestGetState:
    def test_returns_unknown_when_no_state_received_yet(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.latest_state = None

        # act
        response = client.get("/state")

        # assert
        body = response.json()
        assert response.status_code == 200
        assert body["state"] == GateState.UNKNOWN.value
        assert body["rssi"] is None

    def test_returns_latest_retained_state(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.latest_state = GateStatePayload(
            v=1, state=GateState.CLOSED, rssi=-67, uptime_s=84120, ts=1753036800
        )
        fake_mqtt_client.latest_availability = Availability.ONLINE

        # act
        response = client.get("/state")

        # assert
        body = response.json()
        assert body["state"] == GateState.CLOSED.value
        assert body["controller_online"] is True
        assert body["rssi"] == -67

    def test_reports_offline_controller_alongside_last_known_state(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.latest_state = GateStatePayload(
            v=1, state=GateState.OPEN, rssi=-70, uptime_s=100, ts=1753036800
        )
        fake_mqtt_client.latest_availability = Availability.OFFLINE

        # act
        response = client.get("/state")

        # assert
        assert response.json()["controller_online"] is False


class TestPostTrigger:
    def test_accepted_result_returns_202(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.trigger_result = TriggerAck(
            v=1, request_id="abc123", result=AckResult.ACCEPTED, ts=1753036801
        )

        # act
        response = client.post("/trigger")

        # assert
        assert response.status_code == 202
        assert response.json()["result"] == AckResult.ACCEPTED.value
        assert fake_mqtt_client.last_trigger_source == "app"

    def test_duplicate_result_is_not_an_error(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.trigger_result = TriggerAck(
            v=1, request_id="abc123", result=AckResult.DUPLICATE, ts=1753036801
        )

        # act
        response = client.post("/trigger")

        # assert
        assert response.status_code == 200
        assert response.json()["result"] == AckResult.DUPLICATE.value

    def test_rejected_cooldown_is_not_an_error(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.trigger_result = TriggerAck(
            v=1, request_id="abc123", result=AckResult.REJECTED_COOLDOWN, ts=1753036801
        )

        # act
        response = client.post("/trigger")

        # assert
        assert response.status_code == 200
        assert response.json()["result"] == AckResult.REJECTED_COOLDOWN.value

    def test_ack_timeout_returns_504(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.trigger_result = TriggerTimeoutError("abc123", 5.0)

        # act
        response = client.post("/trigger")

        # assert
        assert response.status_code == 504
        assert response.json()["request_id"] == "abc123"

    def test_not_connected_returns_503(
        self, client: TestClient, fake_mqtt_client: FakeMqttClient
    ) -> None:
        # arrange
        fake_mqtt_client.trigger_result = MqttConnectionError("not connected to the broker")

        # act
        response = client.post("/trigger")

        # assert
        assert response.status_code == 503
