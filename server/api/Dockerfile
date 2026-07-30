# Pinned per docker-kubernetes.md - never `latest`.
FROM python:3.12.7-slim-bookworm AS builder

WORKDIR /build
COPY pyproject.toml ./
COPY src ./src

RUN python -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip install --no-cache-dir --upgrade pip==24.3.1 && \
    pip install --no-cache-dir .

FROM python:3.12.7-slim-bookworm AS runtime

RUN groupadd --gid 1000 gateapi && \
    useradd --uid 1000 --gid gateapi --shell /usr/sbin/nologin --no-create-home gateapi

COPY --from=builder /opt/venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

WORKDIR /app
COPY src ./src
ENV PYTHONPATH=/app/src

USER gateapi

# Binds inside the container's own network namespace; server/docker-compose.yml
# uses network_mode: host so this is, in effect, the Pi's 127.0.0.1 - satisfying
# hard condition 3 (reachable through cloudflared and nothing else). See
# docs/decisions/0004-docker-compose-over-systemd.md.
EXPOSE 8000
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD python -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:8000/health', timeout=2)" || exit 1

CMD ["uvicorn", "gate_api.main:app", "--host", "127.0.0.1", "--port", "8000"]
