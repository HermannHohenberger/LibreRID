# Feldbetrieb über VPS-Broker (Variante B)

Ziel: `mqtt.hoferland.eu` wird von überall erreichbar (Hostinger-VPS),
das Heimnetz bleibt unverändert (Pi-hole-Biegung bleibt, Home-Tester läuft weiter).

```
Feld: RX --WLAN-Hotspot--> Mobilfunk --> VPS :7006/TLS (Auth nötig)
                                        |-> ntfy-Brücke -> Handy (Push)
Haus: Pi-hole biegt mqtt.hoferland.eu -> docker01 (anonym, nur LAN)
```

## 0. Vorbereitung (einmalig, Hermann)

- [ ] VPS-IP notieren: `ip -4 addr show` auf dem VPS (oder Hetzner/Hostinger-Panel)
- [ ] DNS: A-Record `mqtt.hoferland.eu` → VPS-IP setzen (dort, wo die Zone liegt –
      aktuell zeigt sie auf 1&1-Kasserver, also vermutlich IONOS/1&1-Verwaltung)
- [ ] Pi-hole **unverändert lassen**: interne Zone `mqtt.hoferland.eu → 192.168.178.103`
      gewinnt zu Hause, draußen greift der neue A-Record.
- [ ] Firewall VPS: Port 7006/tcp öffnen (ufw allow 7006/tcp), 1883 NICHT öffnen.

## 1. Zertifikat (Let's Encrypt, standalone)

```bash
sudo apt-get install -y certbot
sudo certbot certonly --standalone -d mqtt.hoferland.eu \
  --pre-hook "systemctl stop mosquitto-tmp 2>/dev/null; true" \
  --deploy-hook "docker restart librerid-mosquitto 2>/dev/null; true"
```
(7006 muss dafür frei/temporär der Container laufen; Renewal prüft certbot automatisch.)

## 2. Broker als Docker-Container

`/opt/librerid/mosquitto/mosquitto.conf`:

```
listener 7006
ssl_client_renegotiation_interval 43200
certfile /etc/letsencrypt/live/mqtt.hoferland.eu/fullchain.pem
keyfile  /etc/letsencrypt/live/mqtt.hoferland.eu/privkey.pem
cafile   /etc/letsencrypt/live/mqtt.hoferland.eu/chain.pem

allow_anonymous false
password_file /mosquitto/config/passwd

persistence true
persistence_location /mosquitto/data/
log_dest json-file 2
```

Passwort für den RX anlegen (NUR für RX; eigenes, hier wiederverwendbar):

```bash
cd /opt/librerid/mosquitto
openssl rand -base64 24 | tr -d '\n' > rx-pass.txt   # nur lokal auf dem VPS
docker run --rm -v "$PWD:/w" eclipse-mosquitto:2 \
  sh -c "touch /w/passwd && mosquitto_passwd -b -c /w/passwd librerid-rx \$(cat /w/rx-pass.txt)"
chmod 600 rx-pass.txt passwd
```

`/opt/librerid/docker-compose.yml` (nur der neue Service):

```yaml
services:
  librerid-mosquitto:
    image: eclipse-mosquitto:2
    container_name: librerid-mosquitto
    restart: unless-stopped
    network_mode: host
    volumes:
      - /opt/librerid/mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf:ro
      - /opt/librerid/mosquitto/passwd:/mosquitto/config/passwd:ro
      - /opt/librerid/mosquitto/data:/mosquitto/data:ro
      - /etc/letsencrypt:/etc/letsencrypt:ro
```

Start + Kontrolle:

```bash
cd /opt/librerid && docker compose up -d librerid-mosquitto
docker logs librerid-mosquitto --tail 20
```

## 3. ntfy-Brücke (Push aufs Handy bei jeder Drohne)

`/opt/librerid/bridge/ntfy_bridge.py` siehe `rx/tools/vps_ntfy_bridge.py`
in diesem Repo. Compose-Service:

```yaml
  librerid-bridge:
    image: eclipse-mosquitto:2
    container_name: librerid-bridge
    restart: unless-stopped
    network_mode: host
    entrypoint: >
      sh -c 'mosquitto_sub -h 127.0.0.1 -p 7006 -u librerid-rx
      -P "$(cat /opt/librerid/mosquitto/rx-pass.txt)" -t "librerid/#" -v
      | while IFS= read -r line; do
          topic="$${line%% *}"; payload="$${line#* }";
          curl -s -H "Title: LibreRID" -H "Priority: high"
            -d "$$payload" "https://ntfy.hoferland.cloud/librerid-feld" >/dev/null;
        done'
    volumes:
      - /opt/librerid/mosquitto:/opt/librerid/mosquitto:ro
```
(ntfy-Topic/Server an deine bestehende ntfy-Konfiguration anpassen,
ggf. Access-Token nötig – dann als Header ergänzen.)

## 4. RX-Seite

- `config.h`: `MQTT_USER "librerid-rx"`, `MQTT_PASS "<rx-pass.txt>"`,
  `MQTT_HOST "mqtt.hoferland.eu"` bleibt (Pi-hole siegt intern, VPS extern).
- Heim-Broker ignoriert die Credentials (anonymer Modus) → Heimtests bleiben grün.

## 5. Abnahme im Feld

1. Hotspot an (2,4 GHz / „Maximale Kompatibilität"), VPN aus, Privater Relay aus.
2. RX stromlos 10 s, neu einstecken, warten bis `mqtt: verbunden`.
3. Auf dem VPS: `docker logs librerid-bridge --tail 5` – dort jede Meldung sehen.
4. Handy-Push: ntfy-App, Topic `librerid-feld`.
5. Chupito einfliegen lassen → `seen`-Meldung mit Seriennummer muss bis aufs Handy.
