class GateAPIError(Exception):
    """Base class for every exception raised by gate_api."""


class MqttConnectionError(GateAPIError):
    """Raised when the MQTT broker connection cannot be established or drops."""


class TriggerTimeoutError(GateAPIError):
    """Raised when no gate/cmd/ack arrives for a trigger's request_id in time."""

    def __init__(self, request_id: str, timeout_s: float) -> None:
        self.request_id = request_id
        self.timeout_s = timeout_s
        super().__init__(f"no ack for request_id={request_id} within {timeout_s}s")


class MalformedPayloadError(GateAPIError):
    """Raised when an inbound MQTT payload fails schema or version validation."""

    def __init__(self, topic: str, reason: str) -> None:
        self.topic = topic
        self.reason = reason
        super().__init__(f"malformed payload on {topic}: {reason}")
