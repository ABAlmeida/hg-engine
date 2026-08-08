#!/usr/bin/env python3

import argparse
import re
from pathlib import Path


DEFINE_PATTERN = re.compile(r"^\s*#define\s+([A-Z][A-Z0-9_]*)\s+(.+?)\s*$")
ALLOWED_EXPRESSION_PATTERN = re.compile(r"^[A-Z0-9_xXa-fA-F()+\-*/%<>&|~ \t]+$")
REQUIRED_CONSTANTS = {
    "ITEM_NONE",
    "ITEM_POKE_BAIT",
    "ITEM_SHINY_BAIT",
    "ITEM_HEALING_KIT",
    "ITEM_IV_MAX",
    "MAX_BASE_ITEM_NUM",
    "MAX_TOTAL_ITEM_NUM",
}


def strip_line_comment(value: str) -> str:
    return value.split("//", 1)[0].rstrip()


def generate_constants(header: Path) -> str:
    definitions: list[tuple[str, str]] = []
    seen: set[str] = set()
    collecting = False

    for line_number, line in enumerate(header.read_text(encoding="utf-8").splitlines(), 1):
        match = DEFINE_PATTERN.match(line)
        if match is None:
            continue

        name, expression = match.groups()
        if name == "ITEM_NONE":
            collecting = True
        if not collecting:
            continue
        if not (name.startswith("ITEM_") or name in {"MAX_BASE_ITEM_NUM", "MAX_TOTAL_ITEM_NUM"}):
            continue

        expression = strip_line_comment(expression)
        if not expression or "\\" in expression or not ALLOWED_EXPRESSION_PATTERN.fullmatch(expression):
            raise ValueError(f"Unsupported expression for {name} on line {line_number}: {expression!r}")
        if name in seen:
            raise ValueError(f"Duplicate item constant {name} on line {line_number}")

        definitions.append((name, expression))
        seen.add(name)
        if name == "MAX_TOTAL_ITEM_NUM":
            break

    missing = REQUIRED_CONSTANTS - seen
    if missing:
        raise ValueError(f"Missing required item constants: {', '.join(sorted(missing))}")

    lines = ["// Generated from include/constants/item.h. Do not edit.", ""]
    lines.extend(f".equ {name}, {expression}" for name, expression in definitions)
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Armips item constants from the canonical C header."
    )
    parser.add_argument("header", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    generated = generate_constants(args.header)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
