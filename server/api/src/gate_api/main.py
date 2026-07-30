import asyncio
from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

import structlog
from fastapi import FastAPI

from gate_api.api.routes import router
from gate_api.config import Settings
from gate_api.logging import configure_logging
from gate_api.mqtt.client import GateMqttClient

logger = structlog.get_logger(__name__)


@asynccontextmanager
async def lifespan(app: FastAPI) -> AsyncGenerator[None]:
    # Required fields are sourced from the environment / .env at runtime;
    # mypy can't see that BaseSettings resolves them outside the constructor.
    settings = Settings()  # type: ignore[call-arg]
    mqtt_client = GateMqttClient(settings)
    app.state.mqtt_client = mqtt_client

    mqtt_task = asyncio.create_task(mqtt_client.run())
    logger.info("gate_api_started")
    try:
        yield
    finally:
        mqtt_client.stop()
        mqtt_task.cancel()
        try:
            await mqtt_task
        except asyncio.CancelledError:
            pass
        logger.info("gate_api_stopped")


def create_app() -> FastAPI:
    configure_logging()
    app = FastAPI(title="gate-api", lifespan=lifespan)
    app.include_router(router)
    return app


app = create_app()
