"""
running this script regenerates the following files based on config.json:

  - config.ino: updates slots array, wifi credentials, server url and secret
  - client.ino: updates the mode of the device

if the device is set to entry mode, it will handle an lcd display and entry
gate along with the slots. if it is set to exit mode, it will handle the exit
gate along with the slots. if it is set to slots mode, it will handle only
the assigned slots.

an example config.json is as follows:

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

the code can then be compiled and deployed to the device using the arduino ide/cli.
"""

import json
import argparse
import re
import sys

def generate_config(json_path, config_ino_path, client_ino_path):
    with open(json_path, 'r') as f:
        cfg = json.load(f)

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

    wifi = cfg.get('wifi', {})
    server = cfg.get('server', {})
    lines.append(f"String ssid = \"{wifi.get('ssid','')}\";\n")
    lines.append(f"String password = \"{wifi.get('password','')}\";\n")
    lines.append(f"String secret = \"{server.get('secret','')}\";\n")
    lines.append(f"String serverUrl = \"{server.get('url','')}\";\n")

    with open(config_ino_path, 'w') as f:
        f.writelines(lines)
    print(f"[OK] Wrote {config_ino_path}")

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
    try:
        generate_config("config.json", "config.ino", "client.ino")
    except Exception as e:
        print(f"[ERROR] {e}")
        sys.exit(1)

