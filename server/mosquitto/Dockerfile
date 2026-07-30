# Pinned per docker-kubernetes.md - never `latest`.
FROM eclipse-mosquitto:2.0.18

# gettext provides envsubst, used to bake MOSQUITTO_LAN_BIND_IP into the
# listener's bind address at container start (mosquitto.conf has no native
# env-var expansion).
RUN apk add --no-cache gettext

COPY mosquitto.conf.template /mosquitto/config/mosquitto.conf.template
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh && \
    chown mosquitto:mosquitto /mosquitto/config/mosquitto.conf.template

USER mosquitto

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD nc -z "$MOSQUITTO_LAN_BIND_IP" 1883 || exit 1

ENTRYPOINT ["/entrypoint.sh"]
