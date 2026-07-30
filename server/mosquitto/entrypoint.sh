#!/bin/sh
# Renders mosquitto.conf from the template using MOSQUITTO_LAN_BIND_IP, then
# hands off to mosquitto. Required because mosquitto.conf has no built-in env
# var substitution (server/mosquitto/README.md).
set -eu

: "${MOSQUITTO_LAN_BIND_IP:?MOSQUITTO_LAN_BIND_IP must be set - see server/.env.example}"

envsubst '${MOSQUITTO_LAN_BIND_IP}' \
    < /mosquitto/config/mosquitto.conf.template \
    > /mosquitto/config/mosquitto.conf

exec mosquitto -c /mosquitto/config/mosquitto.conf
