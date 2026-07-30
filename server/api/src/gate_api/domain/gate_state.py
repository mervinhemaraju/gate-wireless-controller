from enum import StrEnum


class GateState(StrEnum):
    """Mirrors .claude/skills/mqtt-contract/SKILL.md exactly - keep in lockstep
    with the firmware's GateState enum and the app's. Never default to CLOSED:
    an unknown gate must read as UNKNOWN, not as a specific, possibly wrong, state.
    """

    UNKNOWN = "unknown"
    OPEN = "open"
    OPENING = "opening"
    CLOSED = "closed"
    CLOSING = "closing"
    STOPPED = "stopped"
    FAULT = "fault"
