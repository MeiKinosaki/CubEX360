#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
python3 - <<'PY'
import json
from pathlib import Path

paths = [
    Path("frontend/library.json"),
    Path("frontend/settings.json"),
    Path("frontend/filters.json"),
    Path("frontend/per_game_profile.schema.json"),
    Path("assets/boot/boot_sequence.json"),
]
for path in paths:
    with path.open("r", encoding="utf-8") as stream:
        data = json.load(stream)
    if not isinstance(data, dict) or "schema_version" not in data:
        raise SystemExit(f"{path}: expected JSON object with schema_version")
    print(f"OK {path}")

settings = json.loads(Path("frontend/settings.json").read_text(encoding="utf-8"))
profiles = settings.get("profiles", {})
for required in ("quality", "balanced", "performance"):
    if required not in profiles:
        raise SystemExit(f"frontend/settings.json: missing profile {required}")

filters = json.loads(Path("frontend/filters.json").read_text(encoding="utf-8"))
if not filters.get("available_filters"):
    raise SystemExit("frontend/filters.json: available_filters must not be empty")
print("CubEX360 frontend configuration validation: PASS")
PY
