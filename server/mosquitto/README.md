# Mosquitto Broker

Bound to the Pi's LAN interface (`MOSQUITTO_LAN_BIND_IP` in `server/.env`),
never to `0.0.0.0` and never proxied through the Cloudflare Tunnel. Only the
ESP32 (on the home network) and the `gate-api` container (via
`network_mode: host`, see `docs/decisions/0004-docker-compose-over-systemd.md`)
can reach it.

## Generating the Password File

`allow_anonymous false` per the MQTT contract - every client authenticates.
The password file is gitignored (`server/mosquitto/passwords/`) and generated
once, locally, before first start:

```sh
mkdir -p passwords
docker run --rm -v "$(pwd)/passwords:/mosquitto/passwords" eclipse-mosquitto:2.0.18 \
    mosquitto_passwd -c -b /mosquitto/passwords/passwordfile gate-api "<gate-api password>"
docker run --rm -v "$(pwd)/passwords:/mosquitto/passwords" eclipse-mosquitto:2.0.18 \
    mosquitto_passwd -b /mosquitto/passwords/passwordfile gate-esp32 "<gate-esp32 password>"
```

Use `-c` only on the first client (it (re)creates the file); every
subsequent client uses `-b` alone or the file is overwritten.

The `gate-api` password must match `MQTT_API_PASSWORD` in `server/.env`. The
`gate-esp32` password must match the value flashed into the ESP32's
`firmware/include/secrets.h` (never committed either).

## Client IDs

Distinct, stable client IDs per the contract - duplicate IDs cause a silent
reconnect loop as each client kicks the other off:

- `gate-api` - the FastAPI service
- `gate-esp32` - the firmware
