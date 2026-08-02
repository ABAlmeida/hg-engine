#!/usr/bin/env python3

import argparse
import re
from pathlib import Path


def is_defined(source: str, name: str) -> bool:
    pattern = rf"^[ \t]*#[ \t]*define[ \t]+{re.escape(name)}(?:[ \t]|$)"
    return re.search(pattern, source, flags=re.MULTILINE) is not None


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Armips feature settings from the C configuration."
    )
    parser.add_argument("config", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    config = args.config.read_text(encoding="utf-8")
    bait_enabled = int(is_defined(config, "IMPLEMENT_BAIT_ENCOUNTERS"))
    reusable_healer_enabled = int(is_defined(config, "IMPLEMENT_REUSABLE_HEALER"))
    revised_opening_enabled = int(is_defined(config, "IMPLEMENT_REVISED_OPENING"))
    generated = (
        "// Generated from include/config.h. Do not edit.\n"
        f"IMPLEMENT_BAIT_ENCOUNTERS equ {bait_enabled}\n"
        f"IMPLEMENT_REUSABLE_HEALER equ {reusable_healer_enabled}\n"
        f"IMPLEMENT_REVISED_OPENING equ {revised_opening_enabled}\n"
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
