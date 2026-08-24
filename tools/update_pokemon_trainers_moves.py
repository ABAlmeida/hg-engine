#!/usr/bin/env python3
"""Update effective movesets in the Heartless Gold trainer tracker."""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass

from generate_pokemon_availability import (
    ROOT,
    find_matching_brace,
    indexed_blocks,
    named_field_block,
    parse_moves,
    parse_species,
    read,
)


OUTPUT = ROOT / "documentation" / "POKEMON_TRAINERS.md"
MOVE_SUFFIX_RE = re.compile(r"\s+— Moves:.*$")
UNUSABLE_FLAG_NAMES = (
    "FLAG_UNUSED_MOVE",
    "FLAG_UNUSABLE_IN_GEN_8",
    "FLAG_UNUSABLE_IN_GEN_9",
    "FLAG_UNUSABLE_UNIMPLEMENTED",
)


@dataclass(frozen=True)
class TrainerPokemon:
    species: str
    level: int
    moves: tuple[str, ...]


def initializer_entries(block: str) -> list[str]:
    """Return the direct child initializers from a braced initializer."""
    entries = []
    cursor = 1
    while True:
        opening = block.find("{", cursor)
        if opening == -1:
            return entries
        closing = find_matching_brace(block, opening)
        entries.append(block[opening : closing + 1])
        cursor = closing + 1


def parse_unusable_moves() -> set[str]:
    if not re.search(
        r"^\s*#define\s+BLOCK_LEARNING_UNIMPLEMENTED_MOVES\b",
        read("include/config.h"),
        re.MULTILINE,
    ):
        return set()

    unusable = set()
    for move, block in indexed_blocks(read("data/Moves.c"), r"MOVE_[A-Z0-9_]+"):
        if any(flag in block for flag in UNUSABLE_FLAG_NAMES):
            unusable.add(move)
    return unusable


def default_moves(
    species: str,
    level: int,
    learnsets: dict[str, dict],
    unusable_moves: set[str],
) -> tuple[str, ...]:
    learnset = learnsets.get(species)
    if learnset is None:
        raise ValueError(f"No level-up learnset exists for {species}")

    moves: list[str] = []
    for entry in learnset.get("LevelMoves", []):
        move = entry["Move"]
        if entry["Level"] > level or move in unusable_moves:
            continue
        if move in moves:
            continue
        if len(moves) == 4:
            moves.pop(0)
        moves.append(move)
    return tuple(moves)


def parse_trainer_parties() -> dict[int, list[TrainerPokemon]]:
    learnsets = json.loads(read("data/learnsets/learnsets.json"))
    unusable_moves = parse_unusable_moves()
    trainers = {}

    for trainer_id_text, trainer_block in indexed_blocks(
        read("data/Trainers.c"), r"\d+"
    ):
        data_block = named_field_block(trainer_block, "data")
        party_block = named_field_block(trainer_block, "party")
        if data_block is None or party_block is None:
            continue

        has_custom_moves = "TRAINER_DATA_TYPE_MOVES" in data_block
        party = []
        for pokemon_block in initializer_entries(party_block):
            species_match = re.search(
                r"\.species\s*=\s*(?:MON_WITH_FORM\(\s*)?(SPECIES_[A-Z0-9_]+)",
                pokemon_block,
            )
            level_match = re.search(r"\.level\s*=\s*(\d+)", pokemon_block)
            if species_match is None or level_match is None:
                raise ValueError(f"Trainer {trainer_id_text} has an incomplete party entry")

            species = species_match.group(1)
            level = int(level_match.group(1))
            if has_custom_moves:
                moves_block = named_field_block(pokemon_block, "moves")
                if moves_block is None:
                    raise ValueError(
                        f"Trainer {trainer_id_text} uses custom moves but {species} has no moves field"
                    )
                moves = tuple(
                    move
                    for move in re.findall(r"\bMOVE_[A-Z0-9_]+\b", moves_block)
                    if move != "MOVE_NONE" and move not in unusable_moves
                )
            else:
                moves = default_moves(species, level, learnsets, unusable_moves)
            party.append(TrainerPokemon(species, level, moves))
        trainers[int(trainer_id_text)] = party

    return trainers


def render_moves(moves: tuple[str, ...], move_names: dict[str, str]) -> str:
    if not moves:
        return "None"
    return ", ".join(
        move_names.get(
            move,
            move.removeprefix("MOVE_").replace("_", " ").title(),
        )
        for move in moves
    )


def update_standard_row(
    line: str,
    trainers: dict[int, list[TrainerPokemon]],
    move_names: dict[str, str],
) -> str:
    cells = line.split("|")
    if len(cells) != 7 or not cells[2].strip().isdigit():
        return line

    trainer_id = int(cells[2].strip())
    source_party = trainers.get(trainer_id)
    if source_party is None:
        raise ValueError(f"Documented trainer ID {trainer_id} does not exist in data/Trainers.c")

    documented_party = re.split(r"(?:;\s*|<br>)", cells[5].strip())
    if len(documented_party) != len(source_party):
        raise ValueError(
            f"Trainer {trainer_id} has {len(documented_party)} documented party entries "
            f"but {len(source_party)} source entries"
        )

    rendered_party = []
    for documented, pokemon in zip(documented_party, source_party):
        documented = MOVE_SUFFIX_RE.sub("", documented.strip())
        rendered_party.append(f"{documented} — Moves: {render_moves(pokemon.moves, move_names)}")
    cells[5] = " " + "<br>".join(rendered_party) + " "
    return "|".join(cells)


def update_silver_table(
    lines: list[str],
    trainers: dict[int, list[TrainerPokemon]],
    move_names: dict[str, str],
) -> None:
    variants = [trainers[495][0], trainers[496][0], trainers[497][0]]
    moves_by_species = {
        pokemon.species: render_moves(pokemon.moves, move_names)
        for pokemon in variants
    }
    species_records = parse_species()
    rendered = "<br>".join(
        f"{species_records[species].name}: {moves_by_species[species]}"
        for species in ("SPECIES_CHIKORITA", "SPECIES_CYNDAQUIL", "SPECIES_TOTODILE")
    )

    for index, line in enumerate(lines):
        if line in {
            "| Species | Level | Held item |",
            "| Species | Level | Held item | Moves |",
        }:
            lines[index] = "| Species | Level | Held item | Moves |"
            lines[index + 1] = "|---|---:|---|---|"
            row = lines[index + 2].split("|")
            if "Chikorita, Cyndaquil, or Totodile" not in lines[index + 2]:
                raise ValueError("The Silver 1 party row has an unexpected format")
            if len(row) == 5:
                row.insert(4, f" {rendered} ")
            elif len(row) == 6:
                row[4] = f" {rendered} "
            else:
                raise ValueError("The Silver 1 party row has an unexpected column count")
            lines[index + 2] = "|".join(row)
            return
    raise ValueError("Could not locate the Silver 1 party table")


def generate(current: str) -> str:
    trainers = parse_trainer_parties()
    move_names, _ = parse_moves()
    lines = current.splitlines()

    update_silver_table(lines, trainers, move_names)
    lines = [update_standard_row(line, trainers, move_names) for line in lines]

    marker = "editing workflow."
    for index, line in enumerate(lines):
        if line == marker:
            note = [
                "",
                "Moves are the effective in-game movesets. For trainers without explicit",
                "`.moves` data, this tracker applies the same initialization rules as the game:",
                "it uses implemented level-up moves available at the configured level, ignores",
                "duplicates, and retains the four most recently learned moves.",
            ]
            if index + 2 >= len(lines) or "effective in-game movesets" not in lines[index + 2]:
                lines[index + 1 : index + 1] = note
            break

    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if the tracker movesets are stale",
    )
    args = parser.parse_args()

    current = OUTPUT.read_text(encoding="utf-8")
    generated = generate(current)
    if args.check:
        if current != generated:
            print(f"{OUTPUT.relative_to(ROOT)} has stale trainer movesets.", file=sys.stderr)
            return 1
        print(f"{OUTPUT.relative_to(ROOT)} trainer movesets are current.")
        return 0

    OUTPUT.write_text(generated, encoding="utf-8", newline="\n")
    print(f"Updated movesets in {OUTPUT.relative_to(ROOT)}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
