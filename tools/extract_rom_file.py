#!/usr/bin/env python3

import argparse
from pathlib import Path

import ndspy.rom


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Extract one NitroFS file from a Nintendo DS ROM."
    )
    parser.add_argument("rom", type=Path)
    parser.add_argument("nitrofs_path")
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    rom = ndspy.rom.NintendoDSRom(args.rom.read_bytes())
    file_id = rom.filenames[args.nitrofs_path]

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(rom.files[file_id])


if __name__ == "__main__":
    main()
