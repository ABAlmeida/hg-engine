#!/usr/bin/env python3
"""Add Kanto cave blockers and badge-gated guard triggers."""

import argparse
import struct
from pathlib import Path


FLAG_MAPTEMP_03F = 0x3F
VAR_TEMP_400F = 0x400F


def split_events(path: Path, expected_counts: tuple[int, int, int, int]):
    data = path.read_bytes()
    sections = []
    offset = 0
    for expected, size in zip(expected_counts, (20, 32, 12, 16)):
        count = struct.unpack_from("<I", data, offset)[0]
        if count != expected:
            raise ValueError(f"{path}: expected {expected} events at {offset}, got {count}")
        start = offset + 4
        offset = start + count * size
        sections.append(data[start:offset])
    if offset != len(data):
        raise ValueError(f"{path}: unexpected event-member length")
    return sections


def write_events(path: Path, counts, sections):
    path.write_bytes(b"".join(
        struct.pack("<I", count) + section
        for count, section in zip(counts, sections)
    ))


def add_cave_blocker(path: Path):
    # Runs after patch_removed_snorlax_events.py: IDs 4 and 7-9 are absent.
    sections = split_events(path, (3, 6, 3, 0))
    objects = sections[1]
    ids = {struct.unpack_from("<H", objects, offset)[0]
           for offset in range(0, len(objects), 32)}
    if ids != {0, 1, 2, 3, 5, 6}:
        raise ValueError(f"{path}: unexpected retained Route 11 object IDs")
    # The third warp is the cave entrance. Stand one tile south of it, on
    # the approach formerly occupied by the Snorlax roadblock, rather than
    # inside the cave doorway.
    if struct.unpack_from("<HH", sections[2], 24) != (1354, 302):
        raise ValueError(f"{path}: unexpected Diglett's Cave entrance")
    sections[1] += struct.pack(
        "<16H", 4, 40, 0, 0, FLAG_MAPTEMP_03F, 1, 1, 0,
        0, 0, 0, 0, 1354, 303, 0, 0,
    )
    write_events(path, (3, 7, 3, 0), sections)


def add_guard_trigger(path: Path, warps, guard_script, trigger,
                      object_count=1, guard_index=0, new_guard_script=None):
    sections = split_events(path, (0, object_count, 2, 0))
    guard_offset = guard_index * 32
    if struct.unpack_from("<H", sections[1], guard_offset)[0] != guard_index:
        raise ValueError(f"{path}: unexpected guard object ID")
    if struct.unpack_from("<H", sections[1], guard_offset + 10)[0] != guard_script:
        raise ValueError(f"{path}: unexpected guard script ID")
    positions = tuple(struct.unpack_from("<HH", sections[2], i * 12)
                      for i in range(2))
    if positions != warps:
        raise ValueError(f"{path}: unexpected gatehouse exits {positions}")
    if new_guard_script is not None:
        objects = bytearray(sections[1])
        struct.pack_into("<H", objects, guard_offset + 10, new_guard_script)
        sections[1] = bytes(objects)
    # Cover the complete 32-tile map row/column, including any side lanes.
    # Script direction checks permit travel back toward the arrival route.
    script, x, z, width, height = trigger
    sections[3] = struct.pack("<8H", script, x, z, width, height, 0, 0, VAR_TEMP_400F)
    write_events(path, (0, object_count, 2, 1), sections)


def add_mt_moon_blocker(path: Path):
    sections = split_events(path, (8, 19, 2, 0))
    ids = {struct.unpack_from("<H", sections[1], offset)[0]
           for offset in range(0, len(sections[1]), 32)}
    if ids != set(range(19)):
        raise ValueError(f"{path}: unexpected Route 3 object IDs")
    if struct.unpack_from("<HH", sections[2], 0) != (1172, 102):
        raise ValueError(f"{path}: unexpected Mt. Moon entrance")
    # Stand in front of the doorway, one tile south of its warp.
    sections[1] += struct.pack(
        "<16H", 19, 40, 0, 0, FLAG_MAPTEMP_03F, 6, 1, 0,
        0, 0, 0, 0, 1172, 103, 0, 0,
    )
    write_events(path, (8, 20, 2, 0), sections)


def add_rock_tunnel_blocker(path: Path):
    sections = split_events(path, (3, 2, 3, 1))
    ids = {struct.unpack_from("<H", sections[1], offset)[0]
           for offset in range(0, len(sections[1]), 32)}
    if ids != {0, 1}:
        raise ValueError(f"{path}: unexpected Route 10 object IDs")
    if struct.unpack_from("<HH", sections[2], 0) != (1419, 163):
        raise ValueError(f"{path}: unexpected Rock Tunnel entrance")
    # Stand outside the north entrance, preserving the Power Plant's warp
    # and coordinate event along with both existing NPCs.
    sections[1] += struct.pack(
        "<16H", 2, 40, 0, 0, FLAG_MAPTEMP_03F, 8, 1, 0,
        0, 0, 0, 0, 1419, 164, 0, 0,
    )
    write_events(path, (3, 3, 3, 1), sections)


def add_lavender_gate(path: Path):
    sections = split_events(path, (4, 4, 7, 0))
    objects = bytearray(sections[1])
    if (struct.unpack_from("<H", objects, 0)[0],
            struct.unpack_from("<HH", objects, 24)) != (0, (1415, 252)):
        raise ValueError(f"{path}: unexpected southern Lavender NPC")
    # Move the southern NPC onto the Route 12 approach, immediately north
    # of the interception row, so the guard is visible where players stop.
    struct.pack_into("<H", objects, 2, 34)
    struct.pack_into("<H", objects, 4, 0)
    struct.pack_into("<H", objects, 8, FLAG_MAPTEMP_03F)
    struct.pack_into("<H", objects, 10, 9)
    struct.pack_into("<HH", objects, 24, 1423, 254)
    sections[1] = bytes(objects)
    # Source map matrix: Lavender is cell (44,7), Route 12 starts at (44,8).
    # Intercept the last town row (255) across its complete 32-tile width.
    sections[3] = struct.pack("<8H", 10, 1408, 255, 32, 1, 0, 0, VAR_TEMP_400F)
    write_events(path, (4, 4, 7, 1), sections)


def patch_route_16_gate(path: Path):
    sections = split_events(path, (0, 1, 2, 2))
    if (struct.unpack_from("<H", sections[1], 0)[0],
            struct.unpack_from("<H", sections[1], 10)[0]) != (0, 1):
        raise ValueError(f"{path}: unexpected Route 16 guard")
    positions = tuple(struct.unpack_from("<HH", sections[2], i * 12)
                      for i in range(2))
    if positions != ((10, 7), (1, 7)):
        raise ValueError(f"{path}: unexpected Route 16 gate exits")
    old_coords = (struct.unpack_from("<8H", sections[3], 0),
                  struct.unpack_from("<8H", sections[3], 16))
    if old_coords != ((2, 5, 6, 1, 3, 0, 0, 0x4129),
                      (3, 6, 6, 1, 3, 0, 1, 0x412A)):
        raise ValueError(f"{path}: unexpected Cycling Road triggers")
    sections[3] = (struct.pack("<8H", 2, 7, 0, 1, 32, 0, 0, VAR_TEMP_400F)
                   + struct.pack("<8H", 3, 5, 6, 1, 3, 0, 0, 0x400E))
    write_events(path, (0, 1, 2, 2), sections)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("route_11", type=Path)
    parser.add_argument("route_11_gate", type=Path)
    parser.add_argument("route_6_gate", type=Path)
    parser.add_argument("route_2_gate", type=Path)
    parser.add_argument("forest_north_gate", type=Path)
    parser.add_argument("route_3", type=Path)
    parser.add_argument("route_5_gate", type=Path)
    parser.add_argument("route_10", type=Path)
    parser.add_argument("lavender", type=Path)
    parser.add_argument("route_16_gate", type=Path)
    parser.add_argument("route_15_gate", type=Path)
    args = parser.parse_args()
    add_cave_blocker(args.route_11)
    add_guard_trigger(args.route_11_gate, ((1, 7), (10, 7)), 2, (1, 7, 0, 1, 32))
    add_guard_trigger(args.route_6_gate, ((5, 12), (5, 2)), 1, (2, 0, 8, 32, 1))
    add_guard_trigger(args.route_2_gate, ((5, 2), (5, 12)), 2, (4, 0, 8, 32, 1),
                      object_count=2, guard_index=1, new_guard_script=3)
    add_guard_trigger(args.forest_north_gate, ((5, 12), (5, 2)), 1,
                      (2, 0, 8, 32, 1))
    add_mt_moon_blocker(args.route_3)
    add_guard_trigger(args.route_5_gate, ((5, 12), (5, 2)), 1,
                      (2, 0, 8, 32, 1))
    add_rock_tunnel_blocker(args.route_10)
    add_lavender_gate(args.lavender)
    patch_route_16_gate(args.route_16_gate)
    add_guard_trigger(args.route_15_gate, ((1, 7), (10, 7)), 1,
                      (2, 7, 0, 1, 32))


if __name__ == "__main__":
    main()
