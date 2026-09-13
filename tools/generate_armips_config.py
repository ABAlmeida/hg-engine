#!/usr/bin/env python3

import argparse
import re
from pathlib import Path


def is_defined(source: str, name: str) -> bool:
    pattern = rf"^[ \t]*#[ \t]*define[ \t]+{re.escape(name)}(?:[ \t]|$)"
    return re.search(pattern, source, flags=re.MULTILINE) is not None


def get_boolean_define(source: str, name: str) -> int:
    pattern = rf"^[ \t]*#[ \t]*define[ \t]+{re.escape(name)}[ \t]+(TRUE|FALSE|0|1)[ \t]*(?://.*)?$"
    match = re.search(pattern, source, flags=re.MULTILINE)
    if match is None:
        return 0
    return int(match.group(1) in ("TRUE", "1"))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Armips feature settings from the C configuration."
    )
    parser.add_argument("config", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--debug-config", type=Path)
    args = parser.parse_args()

    config = args.config.read_text(encoding="utf-8")
    debug_config = (
        args.debug_config.read_text(encoding="utf-8")
        if args.debug_config is not None
        else ""
    )
    bait_enabled = int(is_defined(config, "IMPLEMENT_BAIT_ENCOUNTERS"))
    reusable_healer_enabled = int(is_defined(config, "IMPLEMENT_REUSABLE_HEALER"))
    laptop_enabled = int(is_defined(config, "IMPLEMENT_LAPTOP"))
    revised_opening_enabled = int(is_defined(config, "IMPLEMENT_REVISED_OPENING"))
    new_game_info_menu_skipped = int(is_defined(config, "SKIP_NEW_GAME_INFO_MENU"))
    trainer_line_of_sight_disabled = int(
        is_defined(config, "DISABLE_TRAINER_LINE_OF_SIGHT")
    )
    legendary_sanctuary_enabled = int(
        is_defined(config, "IMPLEMENT_LEGENDARY_SANCTUARY")
    )
    sanctuary_content_ready = get_boolean_define(config, "SANCTUARY_CONTENT_READY")
    debug_cheats_enabled = int(is_defined(debug_config, "DEBUG_CHEATS"))
    generated = (
        "// Generated from include/config.h. Do not edit.\n"
        f"IMPLEMENT_BAIT_ENCOUNTERS equ {bait_enabled}\n"
        f"IMPLEMENT_REUSABLE_HEALER equ {reusable_healer_enabled}\n"
        f"IMPLEMENT_LAPTOP equ {laptop_enabled}\n"
        f"IMPLEMENT_REVISED_OPENING equ {revised_opening_enabled}\n"
        f"SKIP_NEW_GAME_INFO_MENU equ {new_game_info_menu_skipped}\n"
        f"DISABLE_TRAINER_LINE_OF_SIGHT equ {trainer_line_of_sight_disabled}\n"
        f"IMPLEMENT_LEGENDARY_SANCTUARY equ {legendary_sanctuary_enabled}\n"
        f"SANCTUARY_CONTENT_READY equ {sanctuary_content_ready}\n"
        f"DEBUG_CHEATS equ {debug_cheats_enabled}\n"
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
