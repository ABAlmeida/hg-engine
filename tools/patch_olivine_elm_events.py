#!/usr/bin/env python3
"""Add the Lighthouse worker and Elm to Olivine City's event member."""

import argparse
import struct
from pathlib import Path


OBJECT_COUNT_OFFSET = 0x7C
OBJECTS_OFFSET = 0x80
WARP_COUNT_OFFSET = 0x1A0
COORD_COUNT_OFFSET = 0x210
COORDS_OFFSET = 0x214
OLIVINE_ELM_MAPTEMP_FLAG = 0x3F
FLAG_GOT_TM01_FROM_CHUCK = 186
STD_LIGHTHOUSE_CONSTRUCTION_WORKER = 2077


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("event_member", type=Path)
    args = parser.parse_args()

    data = args.event_member.read_bytes()
    if len(data) != 0x224:
        raise ValueError("unexpected Olivine City event-member size")
    if u32(data, 0) != 6:
        raise ValueError("unexpected Olivine City background-event count")
    if u32(data, OBJECT_COUNT_OFFSET) != 9:
        raise ValueError("unexpected Olivine City object-event count")
    if u32(data, WARP_COUNT_OFFSET) != 9:
        raise ValueError("unexpected Olivine City warp-event count")
    if u32(data, COORD_COUNT_OFFSET) != 1:
        raise ValueError("unexpected Olivine City coordinate-event count")
    # The final vanilla object is Route 40's connected-map continuation for
    # Cameron. Its unset script ID makes eventFlag 0x5E the originating map ID,
    # so it must remain intact for map-object reconciliation at the boundary.
    if (
        u16(data, 0x180),
        u16(data, 0x182),
        u16(data, 0x188),
        u16(data, 0x18A),
        u16(data, 0x198),
        u16(data, 0x19A),
    ) != (5, 328, 0x5E, 0xFFFF, 252, 271):
        raise ValueError("unexpected Route 40 connected-map object")

    backgrounds = data[:OBJECT_COUNT_OFFSET]
    objects = data[OBJECTS_OFFSET:WARP_COUNT_OFFSET]
    warps = data[WARP_COUNT_OFFSET + 4:COORD_COUNT_OFFSET]
    coords = data[COORDS_OFFSET:]

    # Object 8: construction worker blocking the Lighthouse entrance. Chuck
    # sets this persistent flag during the Storm Badge reward sequence, so the
    # worker is absent when Olivine loads after the Cianwood Gym.
    worker_object = struct.pack(
        "<16H",
        8, 40, 0, 0, FLAG_GOT_TM01_FROM_CHUCK,
        STD_LIGHTHOUSE_CONSTRUCTION_WORKER, 1, 0,
        0, 0, 0, 0, 301, 264, 0, 0,
    )

    # Object 9: Professor Elm, three tiles east of the Gym exit. Map-temporary
    # flag 0x3F is initialized by script entry 12 before objects become active.
    # Unlike the 0x4000 script-scratch flags, map-temporary flags are reset on
    # every map transition and are the established object-visibility mechanism.
    elm_object = struct.pack(
        "<16H",
        9, 99, 0, 0, OLIVINE_ELM_MAPTEMP_FLAG, 1, 3, 0,
        0, 0, 0, 0, 272, 239, 0, 0,
    )
    patched = b"".join(
        (
            backgrounds,
            struct.pack("<I", 11),
            objects,
            worker_object,
            elm_object,
            struct.pack("<I", 9),
            warps,
            struct.pack("<I", 1),
            coords,
        )
    )
    args.event_member.write_bytes(patched)


if __name__ == "__main__":
    main()
