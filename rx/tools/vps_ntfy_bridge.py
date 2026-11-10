#!/usr/bin/env python3
"""LibreRID VPS-Bruecke: MQTT (librerid/#) -> ntfy-Push.

Betrieb auf dem VPS, z. B. in einem eigenen Container mit
python:3.12-slim + paho-mqtt, oder als systemd-Dienst.

Umgebung:
  MQTT_HOST  (Standard: 127.0.0.1)
  MQTT_PORT  (Standard: 7006)
  MQTT_USER / MQTT_PASS   (RX-Credentials vom Broker)
  NTFY_URL   (Standard: https://ntfy.hoferland.cloud)
  NTFY_TOPIC (Standard: librerid-feld)
  NTFY_TOKEN (optional, 'at_...' Access-Token)
  TLS=1      (wenn 1, Port 8883/7006 mit TLS; Standard 1)
"""
import json
import os
import ssl
import sys
import urllib.request

import paho.mqtt.client as mqtt

HOST = os.environ.get("MQTT_HOST", "127.0.0.1")
PORT = int(os.environ.get("MQTT_PORT", "7006"))
USER = os.environ.get("MQTT_USER", "")
PW = os.environ.get("MQTT_PASS", "")
TLS = os.environ.get("TLS", "1") == "1"
NTFY = os.environ.get("NTFY_URL", "https://ntfy.hoferland.cloud")
TOPIC = os.environ.get("NTFY_TOPIC", "librerid-feld")
TOKEN = os.environ.get("NTFY_TOKEN", "")


def push(title: str, message: str, priority: str = "high", tags: str = "small_airplane"):
    req = urllib.request.Request(
        NTFY.rstrip("/") + "/" + TOPIC,
        data=message.encode(),
        headers={
            "Title": title,
            "Priority": priority,
            "Tags": tags,
            **({"Authorization": "Bearer " + TOKEN} if TOKEN else {}),
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=10) as r:
            r.read()
    except Exception as e:  # noqa: BLE001 – Push darf nie den Bruecker legen
        print(f"ntfy-Fehler: {e}", file=sys.stderr)


def on_message(_c, _u, msg):
    try:
        d = json.loads(msg.payload.decode())
    except (ValueError, UnicodeDecodeError):
        return
    t = d.get("t")
    if t == "seen":
        push("Drohne gesichtet",
             f"ID {d.get('id','?')}  MAC {d.get('mac','?')}  RSSI {d.get('rssi','?')} dBm")
    elif t == "fix" and d.get("lon") not in (None, 0):
        push("Drohnenposition",
             f"https://www.openstreetmap.org?mlat={d.get('lat')}&mlon={d.get('lon')}"
             f"\nID {d.get('id','?')}", priority="default")
    elif t == "gone":
        push("Drohne verschwunden", f"ID {d.get('id','?')}", priority="low", tags="house")


def on_connect(c, _u, _f, rc, _p=None):
    print(f"connect rc={rc}", flush=True)
    c.subscribe("librerid/#")


def main():
    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    if USER:
        c.username_pw_set(USER, PW)
    if TLS:
        c.tls_set(cert_reqs=ssl.CERT_NONE)  # Selbst-/LE-Zert: hier pragmatisch
    c.on_connect, c.on_message = on_connect, on_message
    c.connect(HOST, PORT, 30)
    print(f"lausche librerid/# auf {HOST}:{PORT}", flush=True)
    c.loop_forever()


if __name__ == "__main__":
    main()
