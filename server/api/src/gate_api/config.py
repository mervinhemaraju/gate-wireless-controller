from pydantic import SecretStr
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    """Server configuration, sourced from environment variables / .env.

    Field names map to the SCREAMING_SNAKE_CASE env vars documented in
    server/.env.example; never hardcode a value here that .env.example
    already documents.
    """

    model_config = SettingsConfigDict(env_file=".env", case_sensitive=False, extra="ignore")

    mqtt_broker_host: str
    mqtt_broker_port: int = 1883
    mqtt_api_client_id: str = "gate-api"
    mqtt_api_username: str
    mqtt_api_password: SecretStr

    gate_trigger_ack_timeout_s: float = 5.0
