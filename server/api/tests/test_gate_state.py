from gate_api.domain.gate_state import GateState

# .claude/skills/mqtt-contract/SKILL.md defines this set exactly - a change
# here without a matching firmware/app change is the kind of drift the
# contract exists to prevent.
CONTRACT_STATES = {"unknown", "open", "opening", "closed", "closing", "stopped", "fault"}


class TestGateState:
    def test_matches_the_mqtt_contract_exactly(self) -> None:
        # act
        actual = {member.value for member in GateState}

        # assert
        assert actual == CONTRACT_STATES

    def test_unknown_is_a_real_member(self) -> None:
        # A gate must never default to CLOSED before its state is decoded.
        assert GateState.UNKNOWN.value == "unknown"
