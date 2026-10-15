#!/usr/bin/env python3
"""Append gate script entries while preserving existing relative bytecode."""

import argparse
import struct
from pathlib import Path


def read_scripts(path: Path, count: int):
    data = path.read_bytes()
    header_size = count * 4 + 2
    if struct.unpack_from("<H", data, count * 4)[0] != 0xFD13:
        raise ValueError(f"{path}: unexpected script entry count")
    targets = [i * 4 + 4 + struct.unpack_from("<I", data, i * 4)[0]
               for i in range(count)]
    if any(target < header_size or target >= len(data) for target in targets):
        raise ValueError(f"{path}: invalid script entry pointer")
    return targets, data[header_size:]


def append_scripts(original: Path, extra: Path, old_count: int, extra_count: int):
    old_targets, old_body = read_scripts(original, old_count)
    extra_targets, extra_body = read_scripts(extra, extra_count)
    header_size = (old_count + extra_count) * 4 + 2
    old_shift = extra_count * 4
    # Keep each body's original alignment, including ApplyMovement targets.
    extra_header_size = extra_count * 4 + 2
    padding = (extra_header_size - header_size - len(old_body)) % 4
    extra_shift = header_size + len(old_body) + padding - extra_header_size
    targets = ([target + old_shift for target in old_targets]
               + [target + extra_shift for target in extra_targets])
    table = b"".join(struct.pack("<I", target - i * 4 - 4)
                     for i, target in enumerate(targets))
    original.write_bytes(table + struct.pack("<H", 0xFD13)
                         + old_body + bytes(padding) + extra_body)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("script_dir", type=Path)
    parser.add_argument("route_2_extra", type=Path)
    parser.add_argument("mt_moon_extra", type=Path)
    parser.add_argument("rock_tunnel_extra", type=Path)
    parser.add_argument("lavender_extra", type=Path)
    args = parser.parse_args()
    append_scripts(args.script_dir / "2_172", args.route_2_extra, 2, 3)
    append_scripts(args.script_dir / "2_175", args.mt_moon_extra, 5, 2)
    append_scripts(args.script_dir / "2_191", args.rock_tunnel_extra, 7, 2)
    append_scripts(args.script_dir / "2_767", args.lavender_extra, 8, 3)


if __name__ == "__main__":
    main()
