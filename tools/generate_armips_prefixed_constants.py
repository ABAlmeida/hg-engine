#!/usr/bin/env python3

import argparse
import re
from pathlib import Path


DEFINE_PATTERN = re.compile(r"^\s*#define\s+([A-Z][A-Z0-9_]*)\s+(.+?)\s*$")
ALLOWED_EXPRESSION_PATTERN = re.compile(r"^[A-Z0-9_xXa-fA-F()+\-*/%<>&|~ \t]+$")


def generate_constants(header: Path, prefix: str, stop_after: str, extra_names: set[str]) -> str:
    definitions: list[tuple[str, str]] = []
    seen: set[str] = set()

    for line_number, line in enumerate(header.read_text(encoding="utf-8").splitlines(), 1):
        match = DEFINE_PATTERN.match(line)
        if match is None:
            continue

        name, expression = match.groups()
        if not name.startswith(prefix) and name not in extra_names:
            continue

        expression = expression.split("//", 1)[0].rstrip()
        if not expression or "\\" in expression or not ALLOWED_EXPRESSION_PATTERN.fullmatch(expression):
            raise ValueError(f"Unsupported expression for {name} on line {line_number}: {expression!r}")
        if name in seen:
            raise ValueError(f"Duplicate constant {name} on line {line_number}")

        definitions.append((name, expression))
        seen.add(name)
        if name == stop_after:
            break

    if stop_after not in seen:
        raise ValueError(f"Missing stop constant {stop_after}")

    lines = [f"// Generated from {header.as_posix()}. Do not edit.", ""]
    lines.extend(f".equ {name}, {expression}" for name, expression in definitions)
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Armips constants sharing a prefix from a canonical C header."
    )
    parser.add_argument("header", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("prefix")
    parser.add_argument("stop_after")
    parser.add_argument("extra_names", nargs="*")
    args = parser.parse_args()

    generated = generate_constants(args.header, args.prefix, args.stop_after, set(args.extra_names))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
