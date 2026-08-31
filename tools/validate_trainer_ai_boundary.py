#!/usr/bin/env python3
from pathlib import Path


source = Path("src/battle/trainer_ai.c").read_text(encoding="utf-8")
forbidden = ("playerActions", "waza_no_select", "executionOrder", "turnOrder")
found = [token for token in forbidden if token in source]
if found:
    raise SystemExit("trainer AI command-blindness violation: " + ", ".join(found))
overlay_calls = ("HandleLoadOverlay", "UnloadOverlayByID", "CanOverlayBeLoaded")
found = [token for token in overlay_calls if token in source]
if found:
    raise SystemExit("unapproved trainer AI overlay dependency: " + ", ".join(found))
generated_include = 'constants/generated/trainer_ai_semantics_generated.h'
consumers = []
for path in Path("src").rglob("*.c"):
    if generated_include in path.read_text(encoding="utf-8", errors="ignore"):
        consumers.append(path.as_posix())
if consumers != ["src/battle/trainer_ai.c"]:
    raise SystemExit("generated trainer AI tables must have one consumer: " + ", ".join(consumers))
for path in Path("src").rglob("*trainer*ai*"):
    if "overlay" in path.name.lower() and path.suffix in {".c", ".ld", ".s"}:
        raise SystemExit(f"unapproved transient trainer AI artifact: {path}")
