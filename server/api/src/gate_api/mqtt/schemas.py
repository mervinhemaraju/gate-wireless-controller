from enum import StrEnum

from pydantic import BaseModel

from gate_api.domain.gate_state import GateState

# .claude/skills/mqtt-contract/SKILL.md - bump only alongside the firmware and
# app in the same change.
SCHEMA_VERSION = 1

TOPIC_AVAILABILITY = "gate/availability"
TOPIC_STATE = "gate/state"
TOPIC_CMD_TRIGGER = "gate/cmd/trigger"
TOPIC_CMD_ACK = "gate/cmd/ack"


class AckResult(StrEnum):
    """Closed set per the contract. Only ACCEPTED represents an actual gate
    movement - the rest are informational, not error conditions."""

    ACCEPTED = "accepted"
    DUPLICATE = "duplicate"
    REJECTED_COOLDOWN = "rejected_cooldown"
    REJECTED_BUSY = "rejected_busy"
    REJECTED_MALFORMED = "rejected_malformed"


class GateStatePayload(BaseModel):
    v: int
    state: GateState
    rssi: int
    uptime_s: int
    ts: int


class TriggerCommand(BaseModel):
    v: int = SCHEMA_VERSION
    request_id: str
    source: str
    ts: int


class TriggerAck(BaseModel):
    v: int
    request_id: str
    result: AckResult
    ts: int


class Availability(StrEnum):
    ONLINE = "online"
    OFFLINE = "offline"
