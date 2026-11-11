# Feldbetrieb über VPS-Broker (Variante B)

Ziel: **`rid.hoferland.eu`** (neu, Zone liegt bei ALL-INKL) zeigt direkt auf den
Hostinger-VPS. Bestehende Namen/Domains bleiben unangetastet.

```
Feld: RX --WLAN-Hotspot--> Mobilfunk --> rid.hoferland.eu :7006/TLS (Auth)
                                            |-> ntfy-Brücke -> Handy (Push)
Haus: RX verbindet sich von innen ebenfalls gegen rid.hoferland.eu (VPS)
      (Heim-Broker auf docker01 bleibt für HA-Experimente unberührt)
```

## 0. Vorbereitung (einmalig, Hermann)

- [ ] VPS-IP notieren: `ip -4 addr show` auf dem VPS (öffentliche Adresse)
- [ ] ALL-INKL DNS-Verwaltung: **A-Record** `rid.hoferland.eu` → VPS-IP
      (TTD/Standard 300 s ist fein; AAAA weglassen, IPv6-Route später prüfen)
- [ ] Firewall VPS: Port 7006/tcp öffnen (ufw allow 7006/tcp), 1883 NICHT öffnen.
- [ ] Test von außerhalb (z. B. Mobilfunk ohne WLAN):
      `dig +short rid.hoferland.eu` muss die VPS-IP liefern.

## 1. Zertifikat (Let's Encrypt, standalone)

```bash
sudo apt-get install -y certbot
sudo certbot certonly --standalone -d rid.hoferland.eu --key-type rsa \
  --pre-hook "systemctl stop mosquitto-tmp 2>/dev/null; true" \
  --deploy-hook "docker restart librerid-mosquitto 2>/dev/null; true"
```
(`--key-type rsa` wichtig: Der ESP hat ISRG **Root X1** im Flash; die RSA-Kette
läuft über X1, die EC-Kette würde über Root X2 gehen und vom ESP nicht gekannt.)
(7006 muss dafür frei/temporär der Container laufen; Renewal prüft certbot automatisch.)

## 2. Broker als Docker-Container

IST- STAND 2026-11-10, live verifiziert: Zertifikat kommt aus **NPM**
(Proxy-Host `rid.hoferland.eu` + Let's Encrypt in der NPM-UI, Zert-ID `npm-14`).
Da NPM die Dateien nur per Symlinks in `archive/` pflegt und der Broker-Leser
(1883) das root-privkey nicht lesen darf, liegt eine **eigentümerkorrigierte
Kopie** unter `/opt/librerid/mosquitto/certs/` (cron `/etc/cron.d/librerid-cert-sync`,
täglich 03:17: zurückkopieren + chown + Container-Neustart).
Mosquitto-Konfig zeigt auf `/mosquitto/certs/*`. WICHTIG: Nach einer
NPM-Zertifikats-NEUAUSSTELLUNG (andere ID als npm-14) muss der cron-Pfad
angepasst werden (`ls /var/lib/docker/volumes/npm_letsencrypt/_data/live/`).

`/opt/librerid/mosquitto/mosquitto.conf`:

```
listener 7006
ssl_client_renegotiation_interval 43200
certfile /etc/letsencrypt/live/rid.hoferland.eu/fullchain.pem
keyfile  /etc/letsencrypt/live/rid.hoferland.eu/privkey.pem
cafile   /etc/letsencrypt/live/rid.hoferland.eu/chain.pem

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

- `config.h`: `MQTT_HOST "rid.hoferland.eu"`, `MQTT_USER "librerid-rx"`,
  `MQTT_PASS "<rx-pass.txt>"`.
- Gelich laut: `ca_isrg_root_x1.h` passt weiterhin (Let's Encrypt hängt an
  ISRG Root X1), keine Firmware-Änderung nötig außer `config.h`.

## 5. Abnahme im Feld

1. Hotspot an (2,4 GHz / „Maximale Kompatibilität"), VPN aus, Privater Relay aus.
2. RX stromlos 10 s, neu einstecken, warten bis `mqtt: verbunden`.
3. Auf dem VPS: `docker logs librerid-bridge --tail 5` – dort jede Meldung sehen.
4. Handy-Push: ntfy-App, Topic `librerid-feld`.
5. Chupito einfliegen lassen → `seen`-Meldung mit Seriennummer muss bis aufs Handy.

## 6. IST 2026-11-11: Push-Kette LIVE, Architektur final

End-to-End bewiesen: Test-Sichtung (id R-1A2B3C4D5E) erschien als Handy-Push.

```
RX (Hotspot) --TLS--> rid.hoferland.eu:7006 (mosquitto)
  -> librerid-bridge (lauscht librerid/#, Payload t=seen|fix|gone)
  -> POST https://rid-ntfy.hoferland.eu/<GEHEIM-TOPIC>
     (rid-ntfy-Container, ntfy latest, 127.0.0.1:2587,
      KEINE auth.db: Geheim-Topic ist das Passwort, anonym publish ok)
  -> ntfy-App (Server rid-ntfy.hoferland.eu, Topic = NTFY_TOPIC aus bridge.env)
```

- TLS/Terminierung rid-ntfy: nginx-proxy-manager (der „openresty"-Prozess ist NPM selbst).
- Configs: /opt/rid-ntfy/{server.yml,compose.yml}; Brücken-Ziel: /opt/librerid/bridge/bridge.env (NTFY_URL + NTFY_TOPIC).
- Topic rotieren: NTFY_TOPIC in bridge.env ändern + `docker compose up -d --force-recreate librerid-bridge` + neu abonnieren.
- alter Flugplatz-ntfy (flugplatz-ntfy, Port 2586): für LibreRID außer Betrieb.
  SEIT 2026-11-11 kaputt für neue Logins (nach Image-Update 2.11→2.37; auth.db-Verdacht).
  Reparatur = Hermanns Aufgabe, Agenten fassen sie nicht an.

Lehren aus der 401-Saga (für künftige Debugging-Runden):
1. Geheimnisse NIE als Literal durchs Chat-Fenster – das Tool verstümmelt
   `$(...)`, `"\n"`, `:"` schon mal. Stattdessen: Datei + stdin-Pipe oder base64-Block.
2. „Login 401" zuerst am laufenden Server-Container selbst prüfen
   (`docker exec <ntfy> ntfy user list` + Debug-Log), bevor irgendwas neu gesetzt wird.
3. Vor jeder Konfig-Arbeit prüfen, WO die Domain wirklich landet
   (`dig +short <domain>` + `curl -sI` Server-Header) – rid-ntfy zeigte wegen
   IONOS-SSL-Zwang wochenlang aufs Blog-Hosting, nicht auf den VPS.
