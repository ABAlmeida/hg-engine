#!/usr/bin/env python3
"""Remove the Route 11/12 Snorlax roadblock objects from field event data."""

import argparse
import struct
from pathlib import Path


BG_EVENT_SIZE = 20
OBJECT_EVENT_SIZE = 32

FLAG_HIDE_ROUTE_11_SNORLAX = 609
FLAG_HIDE_ROUTE_12_SNORLAX = 795

ROUTE_11_EXPECTED_OBJECT_COUNT = 10
ROUTE_12_EXPECTED_OBJECT_COUNT = 19

ROUTE_11_REMOVED_OBJECTS = {
    4: (1354, 304),
    7: (1355, 303),
    8: (1354, 303),
    9: (1355, 304),
}
ROUTE_12_REMOVED_OBJECTS = {
    15: (1411, 306),
    16: (1411, 305),
    17: (1412, 305),
    18: (1412, 306),
}


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def remove_objects(
    event_member: Path,
    expected_count: int,
    expected_flag: int,
    removed_objects: dict[int, tuple[int, int]],
) -> None:
    data = event_member.read_bytes()
    bg_count = u32(data, 0)
    object_count_offset = 4 + bg_count * BG_EVENT_SIZE
    object_count = u32(data, object_count_offset)
    if object_count != expected_count:
        raise ValueError(
            f"{event_member}: expected {expected_count} objects, found {object_count}"
        )

    objects_offset = object_count_offset + 4
    objects_end = objects_offset + object_count * OBJECT_EVENT_SIZE
    if objects_end + 4 > len(data):
        raise ValueError(f"{event_member}: truncated object-event table")

    retained_objects = []
    found_objects = set()
    for index in range(object_count):
        offset = objects_offset + index * OBJECT_EVENT_SIZE
        record = data[offset : offset + OBJECT_EVENT_SIZE]
        object_id = u16(record, 0)
        if object_id not in removed_objects:
            retained_objects.append(record)
            continue

        expected_x, expected_z = removed_objects[object_id]
        event_flag = u16(record, 8)
        x = u16(record, 24)
        z = u16(record, 26)
        if (event_flag, x, z) != (expected_flag, expected_x, expected_z):
            raise ValueError(
                f"{event_member}: unexpected Snorlax object {object_id} "
                f"flag/position {(event_flag, x, z)}"
            )
        found_objects.add(object_id)

    if found_objects != removed_objects.keys():
        missing = sorted(removed_objects.keys() - found_objects)
        raise ValueError(f"{event_member}: missing Snorlax objects {missing}")

    patched = b"".join(
        (
            data[:object_count_offset],
            struct.pack("<I", len(retained_objects)),
            b"".join(retained_objects),
            data[objects_end:],
        )
    )
    event_member.write_bytes(patched)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("route_11_event", type=Path)
    parser.add_argument("route_12_event", type=Path)
    args = parser.parse_args()

    remove_objects(
        args.route_11_event,
        ROUTE_11_EXPECTED_OBJECT_COUNT,
        FLAG_HIDE_ROUTE_11_SNORLAX,
        ROUTE_11_REMOVED_OBJECTS,
    )
    remove_objects(
        args.route_12_event,
        ROUTE_12_EXPECTED_OBJECT_COUNT,
        FLAG_HIDE_ROUTE_12_SNORLAX,
        ROUTE_12_REMOVED_OBJECTS,
    )


if __name__ == "__main__":
    main()
