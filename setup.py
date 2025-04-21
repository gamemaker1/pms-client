#!/usr/bin/env python3
"""
generate_config.py

Usage:
    python generate_config.py config.json path/to/config.ino path/to/client.ino

Reads a JSON configuration file and regenerates your Arduino ESP project files:
  - config.ino: updates slots array, WIFI credentials, server URL/secret
  - client.ino: updates the MODE (#define MODE "entry"/"exit")

Example config.json schema:
{
  "mode": "exit",
  "wifi": {
    "ssid": "hifi",
    "password": "password"
  },
  "server": {
    "url": "http://192.168.12.1:4242/api",
    "secret": "dcead4a0768dfe0a66774759f64b46e2"
  },
  "slots": [
    { "location": "B3", "trig": 14, "echo": 27, "gled": 26, "rled": 25, "threshold": 4 },
    { "location": "B2", "trig": 32, "echo": 33, "gled": 2,  "rled": 4,  "threshold": 3 },
    { "location": "B1", "trig": 17, "echo": 18, "gled": 15, "rled": 16, "threshold": 3 }
  ]
}
"""
import json
import argparse
import re
import sys

def generate_config(json_path, config_ino_path, client_ino_path):
    # Load JSON config
    with open(json_path, 'r') as f:
        cfg = json.load(f)

    # Generate config.ino content
    lines = []
    lines.append("typedef struct Slot { String location; long detected; bool parked; int trig; int echo; int gled; int rled; int threshold; } Slot;\n")
    lines.append("Slot slots[] = {\n")
    for slot in cfg.get('slots', []):
        loc = slot['location']
        trig = slot['trig']
        echo = slot['echo']
        gled = slot['gled']
        rled = slot['rled']
        thresh = slot['threshold']
        lines.append(f"  {{ \"{loc}\", -1, false, {trig}, {echo}, {gled}, {rled}, {thresh} }},\n")
    lines.append("};\n")
    lines.append("int SLOT_COUNT = sizeof(slots) / sizeof(Slot);\n\n")
    # WiFi and server
    wifi = cfg.get('wifi', {})
    server = cfg.get('server', {})
    lines.append(f"String ssid = \"{wifi.get('ssid','')}\";\n")
    lines.append(f"String password = \"{wifi.get('password','')}\";\n")
    lines.append(f"String secret = \"{server.get('secret','')}\";\n")
    lines.append(f"String serverUrl = \"{server.get('url','')}\";\n")

    # Write config.ino
    with open(config_ino_path, 'w') as f:
        f.writelines(lines)
    print(f"[OK] Wrote {config_ino_path}")

    # Patch client.ino for MODE
    mode = cfg.get('mode', 'entry')
    pattern = re.compile(r'^(#define\s+MODE\s+).*$')
    new_line = f'#define MODE "{mode}"'
    updated = []
    with open(client_ino_path, 'r') as f:
        for line in f:
            if pattern.match(line):
                updated.append(new_line)
            else:
                updated.append(line)
    with open(client_ino_path, 'w') as f:
        f.writelines(updated)
    print(f"[OK] Updated MODE in {client_ino_path} to '{mode}'")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Regenerate ESP project config based on JSON')
    parser.add_argument('json', help='Path to config.json')
    parser.add_argument('config_ino', help='Path to config.ino to overwrite')
    parser.add_argument('client_ino', help='Path to client.ino to patch MODE')
    args = parser.parse_args()
    try:
        generate_config(args.json, args.config_ino, args.client_ino)
    except Exception as e:
        print(f"[ERROR] {e}")
        sys.exit(1)

