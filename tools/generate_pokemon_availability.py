#!/usr/bin/env python3
"""Generate the Heartless Gold Pokemon implementation and usage tracker."""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "documentation" / "POKEMON_AVAILABILITY.md"
TM_OUTPUT = ROOT / "documentation" / "TM_REWARD_AVAILABILITY.md"
TM_BEGIN_MARKER = "<!-- BEGIN GENERATED UNUSABLE TM ASSIGNMENTS -->"
TM_END_MARKER = "<!-- END GENERATED UNUSABLE TM ASSIGNMENTS -->"
SPECIES_RE = re.compile(r"\bSPECIES_[A-Z0-9_]+\b")


@dataclass
class SpeciesRecord:
    constant: str
    name: str
    engine_id: int | None
    dex_number: int | None
    abilities: list[str]
    hidden_ability: str | None
    sprite_present: bool
    learnset_present: bool = False
    unimplemented_level_moves: list[str] = field(default_factory=list)
    unimplemented_machine_moves: list[str] = field(default_factory=list)
    unimplemented_tutor_moves: list[str] = field(default_factory=list)
    unimplemented_egg_moves: list[str] = field(default_factory=list)
    status: str = "Ready"
    reasons: list[str] = field(default_factory=list)


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def find_matching_brace(text: str, opening: int) -> int:
    depth = 0
    quote: str | None = None
    escaped = False
    line_comment = False
    block_comment = False
    i = opening
    while i < len(text):
        char = text[i]
        next_char = text[i + 1] if i + 1 < len(text) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                i += 1
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            i += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            i += 1
        elif char in {'"', "'"}:
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError(f"Unmatched opening brace at offset {opening}")


def indexed_blocks(text: str, key_pattern: str) -> list[tuple[str, str]]:
    pattern = re.compile(rf"^\s*\[({key_pattern})\]\s*=\s*\{{", re.MULTILINE)
    result = []
    for match in pattern.finditer(text):
        opening = text.find("{", match.start())
        closing = find_matching_brace(text, opening)
        result.append((match.group(1), text[opening : closing + 1]))
    return result


def named_field_block(text: str, field_name: str) -> str | None:
    match = re.search(rf"\.{re.escape(field_name)}\s*=\s*\{{", text)
    if not match:
        return None
    opening = text.find("{", match.start())
    return text[opening : find_matching_brace(text, opening) + 1]


def named_field_value(text: str, field_name: str) -> str | None:
    block = named_field_block(text, field_name)
    if block:
        return block
    match = re.search(rf"\.{re.escape(field_name)}\s*=\s*([A-Z][A-Z0-9_]*)\s*,", text)
    return match.group(1) if match else None


def decode_c_string(value: str) -> str:
    return value.replace(r"\n", " ").replace(r"\r", " ").replace(r'\"', '"').replace(r"\\", "\\")


def humanize_constant(value: str, prefixes: tuple[str, ...] = ()) -> str:
    for prefix in prefixes:
        if value.startswith(prefix):
            value = value[len(prefix) :]
            break
    words = value.lower().split("_")
    substitutions = {
        "mt": "Mt.",
        "ft": "Ft.",
        "b": "B",
        "f": "F",
        "1f": "1F",
        "2f": "2F",
        "3f": "3F",
        "4f": "4F",
        "5f": "5F",
        "b1f": "B1F",
        "b2f": "B2F",
        "b3f": "B3F",
    }
    return " ".join(substitutions.get(word, word.capitalize()) for word in words)


def markdown_escape(value: str) -> str:
    return value.replace("|", r"\|").replace("\n", " ")


def parse_species_numbers() -> tuple[dict[str, int], dict[str, int]]:
    all_numeric = {}
    for constant, value in re.findall(
        r"^#define\s+(SPECIES_[A-Z0-9_]+)\s+(\d+)(?:\s*//.*)?$",
        read("include/constants/species.h"),
        re.MULTILINE,
    ):
        all_numeric[constant] = int(value)
    maximum_match = re.search(
        r"^#define\s+MAX_CANONICAL_MON_NUM\s+\((SPECIES_[A-Z0-9_]+)\)",
        read("include/constants/species.h"),
        re.MULTILINE,
    )
    if not maximum_match or maximum_match.group(1) not in all_numeric:
        raise ValueError("Could not resolve MAX_CANONICAL_MON_NUM")
    maximum = all_numeric[maximum_match.group(1)]
    engine_ids = {
        constant: number
        for constant, number in all_numeric.items()
        if 1 <= number <= maximum
    }
    reserved_pattern = re.compile(r"SPECIES_(?:EGG|BAD_EGG|\d+)$")
    canonical_constants = [
        constant
        for constant, _ in sorted(engine_ids.items(), key=lambda item: item[1])
        if not reserved_pattern.fullmatch(constant)
    ]
    national_dex = {
        constant: index
        for index, constant in enumerate(canonical_constants, start=1)
    }
    return engine_ids, national_dex


def parse_species() -> dict[str, SpeciesRecord]:
    engine_ids, national_dex = parse_species_numbers()
    sprite_dirs = {path.name.lower() for path in (ROOT / "data/graphics/sprites").iterdir() if path.is_dir()}
    records = {}
    for constant, block in indexed_blocks(read("data/Species.c"), r"SPECIES_[A-Z0-9_]+"):
        if constant == "SPECIES_NONE":
            continue
        if constant in engine_ids and constant not in national_dex:
            continue
        name_match = re.search(r'\.name\s*=\s*"((?:\\.|[^"\\])*)"', block)
        ability_match = re.search(r"\.abilities\s*=\s*\{([^}]+)\}", block)
        name = decode_c_string(name_match.group(1)) if name_match else ""
        if not name or name == "-----":
            name = humanize_constant(constant, ("SPECIES_",))
        abilities = re.findall(r"ABILITY_[A-Z0-9_]+", ability_match.group(1)) if ability_match else []
        sprite_key = constant.removeprefix("SPECIES_").lower()
        records[constant] = SpeciesRecord(
            constant=constant,
            name=name,
            engine_id=engine_ids.get(constant),
            dex_number=national_dex.get(constant),
            abilities=[ability for ability in abilities if ability != "ABILITY_NONE"],
            hidden_ability=None,
            sprite_present=sprite_key in sprite_dirs,
        )
    for constant, ability in re.findall(
        r"\[(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*(ABILITY_[A-Z0-9_]+)",
        read("data/HiddenAbilityTable.c"),
    ):
        if constant in records and ability != "ABILITY_NONE":
            records[constant].hidden_ability = ability
    return records


def parse_moves() -> tuple[dict[str, str], set[str]]:
    names = {}
    unimplemented = set()
    for constant, block in indexed_blocks(read("data/Moves.c"), r"MOVE_[A-Z0-9_]+"):
        name_match = re.search(r'\.name\s*=\s*"((?:\\.|[^"\\])*)"', block)
        names[constant] = (
            decode_c_string(name_match.group(1))
            if name_match
            else humanize_constant(constant, ("MOVE_",))
        )
        humanized = humanize_constant(constant, ("MOVE_",))
        if "_" in constant and names[constant].lower() == humanized.replace(" ", "").lower():
            names[constant] = humanized
        if "FLAG_UNUSABLE_UNIMPLEMENTED" in block:
            unimplemented.add(constant)
    return names, unimplemented


def attach_learnset_findings(
    records: dict[str, SpeciesRecord],
    move_names: dict[str, str],
    unimplemented: set[str],
    tm_items_by_move: dict[str, list[str]],
) -> None:
    learnsets = json.loads(read("data/learnsets/learnsets.json"))
    for constant, learnset in learnsets.items():
        record = records.get(constant)
        if not record:
            continue
        record.learnset_present = True
        level_moves = {
            entry["Move"]
            for entry in learnset.get("LevelMoves", [])
            if isinstance(entry, dict) and entry.get("Move") in unimplemented
        }
        categorized_moves = {}
        for category in ("MachineMoves", "TutorMoves", "EggMoves"):
            moves = set()
            for entry in learnset.get(category, []):
                move = entry.get("Move") if isinstance(entry, dict) else entry
                if move in unimplemented:
                    moves.add(move)
            categorized_moves[category] = moves
        record.unimplemented_level_moves = sorted(
            move_names.get(move, humanize_constant(move, ("MOVE_",)))
            for move in level_moves
        )
        record.unimplemented_machine_moves = sorted(
            (
                f"{move_names.get(move, humanize_constant(move, ('MOVE_',)))} "
                f"({', '.join(tm_items_by_move[move])})"
                if tm_items_by_move.get(move)
                else f"{move_names.get(move, humanize_constant(move, ('MOVE_',)))} "
                "(no current TM item)"
            )
            for move in categorized_moves["MachineMoves"]
        )
        record.unimplemented_tutor_moves = sorted(
            move_names.get(move, humanize_constant(move, ("MOVE_",)))
            for move in categorized_moves["TutorMoves"]
        )
        record.unimplemented_egg_moves = sorted(
            move_names.get(move, humanize_constant(move, ("MOVE_",)))
            for move in categorized_moves["EggMoves"]
        )


def parse_macro_species(text: str) -> dict[str, set[str]]:
    logical_lines = []
    current = ""
    for line in text.splitlines():
        current += line.rstrip("\\").strip() + " "
        if not line.rstrip().endswith("\\"):
            logical_lines.append(current)
            current = ""
    raw = {}
    for line in logical_lines:
        match = re.match(r"\s*#define\s+([A-Z][A-Z0-9_]*)\s+(.+)", line)
        if match and "(" not in match.group(1):
            raw[match.group(1)] = match.group(2)

    resolved: dict[str, set[str]] = {}

    def resolve(name: str, active: set[str]) -> set[str]:
        if name in resolved:
            return resolved[name]
        if name in active or name not in raw:
            return set()
        body = raw[name]
        species = set(SPECIES_RE.findall(body))
        for token in re.findall(r"\b[A-Z][A-Z0-9_]+\b", body):
            if token in raw:
                species.update(resolve(token, active | {name}))
        resolved[name] = species
        return species

    for name in raw:
        resolve(name, set())
    return resolved


def species_in_text(text: str, macros: dict[str, set[str]]) -> set[str]:
    result = set(SPECIES_RE.findall(text))
    for token in re.findall(r"\b[A-Z][A-Z0-9_]+\b", text):
        result.update(macros.get(token, set()))
    result.discard("SPECIES_NONE")
    return result


def parse_ordinary_encounters(disabled_methods: set[str]) -> dict[str, set[str]]:
    text = read("data/Encounters.c")
    macros = parse_macro_species(text)
    locations: dict[str, set[str]] = defaultdict(set)
    method_fields = (
        ("rateWalk", "landSlots", "land"),
        ("rateSurf", "surfSlots", "surf"),
        ("rateRockSmash", "rockSmashSlots", "rock smash"),
        ("rateOldRod", "oldRodSlots", "Fishing Rod"),
        ("rateGoodRod", "goodRodSlots", "Good Rod"),
        ("rateSuperRod", "superRodSlots", "Super Rod"),
    )
    for table, block in indexed_blocks(text, r"ENCDATA_[A-Z0-9_]+"):
        if "UNUSED" in table:
            continue
        raw_location = table.removeprefix("ENCDATA_")
        raw_location = re.sub(r"^[A-Z]\d+(?:R\d+)?_", "", raw_location)
        location = humanize_constant(raw_location)
        for rate_field, slots_field, method in method_fields:
            if slots_field.removesuffix("Slots") in disabled_methods:
                continue
            rate_match = re.search(rf"\.{rate_field}\s*=\s*(\d+)", block)
            slots = named_field_value(block, slots_field)
            if rate_match and int(rate_match.group(1)) > 0 and slots:
                for species in species_in_text(slots, macros):
                    locations[species].add(f"{location} ({method})")

        rate_walk = re.search(r"\.rateWalk\s*=\s*(\d+)", block)
        rate_surf = re.search(r"\.rateSurf\s*=\s*(\d+)", block)
        if rate_walk and int(rate_walk.group(1)) > 0:
            land_swarm = re.search(r"\.landSwarm\s*=\s*(SPECIES_[A-Z0-9_]+)", block)
            if land_swarm:
                for species in species_in_text(land_swarm.group(1), macros):
                    locations[species].add(f"{location} (land swarm)")
        if rate_surf and int(rate_surf.group(1)) > 0:
            surf_swarm = re.search(r"\.surfSwarm\s*=\s*(SPECIES_[A-Z0-9_]+)", block)
            if surf_swarm:
                for species in species_in_text(surf_swarm.group(1), macros):
                    locations[species].add(f"{location} (surf swarm)")
    return locations


def parse_safari_encounters(disabled_methods: set[str]) -> dict[str, set[str]]:
    text = read("data/SafariEncounters.c")
    result: dict[str, set[str]] = defaultdict(set)
    for area, block in indexed_blocks(text, r"SAFARI_ZONE_AREA_[A-Z0-9_]+"):
        location = humanize_constant(area, ("SAFARI_ZONE_AREA_",))
        methods = (
            ("land", "land"),
            ("surf", "surf"),
            ("oldRod", "Fishing Rod"),
            ("goodRod", "Good Rod"),
            ("superRod", "Super Rod"),
        )
        for field_name, method in methods:
            if field_name in disabled_methods:
                continue
            field_block = named_field_block(block, field_name)
            if field_block:
                for species in species_in_text(field_block, {}):
                    result[species].add(f"Safari Zone: {location} ({method})")
    return result


def parse_headbutt_encounters() -> dict[str, set[str]]:
    text = read("data/Headbutt.c")
    archive_match = re.search(r"const\s+HeadbuttArchiveData\s+__data\s*=\s*\{", text)
    if not archive_match:
        return {}
    opening = text.find("{", archive_match.start())
    archive = text[opening : find_matching_brace(text, opening) + 1]
    result: dict[str, set[str]] = defaultdict(set)
    for match in re.finditer(r"^\s*\.([A-Za-z][A-Za-z0-9_]*)\s*=\s*\{", archive, re.MULTILINE):
        field_opening = archive.find("{", match.start())
        block = archive[field_opening : find_matching_brace(archive, field_opening) + 1]
        normal_count = re.search(r"\.normalTreeCount\s*=\s*(\d+)", block)
        special_count = re.search(r"\.specialTreeCount\s*=\s*(\d+)", block)
        if not ((normal_count and int(normal_count.group(1))) or (special_count and int(special_count.group(1)))):
            continue
        field_name = match.group(1)
        type_match = re.search(rf"HeadbuttFile_\d+_([A-Za-z0-9_]+)\s+{re.escape(field_name)}\s*;", text)
        if type_match:
            location = humanize_constant(type_match.group(1).upper())
        else:
            location = re.sub(r"(?<!^)(?=[A-Z])", " ", field_name).replace("_", " ").title()
        for species in species_in_text(block, {}):
            result[species].add(f"{location} (Headbutt)")
    return result


def merge_locations(*sources: dict[str, set[str]]) -> dict[str, set[str]]:
    merged: dict[str, set[str]] = defaultdict(set)
    for source in sources:
        for species, locations in source.items():
            merged[species].update(locations)
    return merged


def parse_trainers() -> dict[str, list[str]]:
    result: dict[str, list[str]] = defaultdict(list)
    for trainer_id, block in indexed_blocks(read("data/Trainers.c"), r"\d+"):
        party = named_field_block(block, "party")
        if not party:
            continue
        name_match = re.search(r'\.name\s*=\s*"((?:\\.|[^"\\])*)"', block)
        name = decode_c_string(name_match.group(1)) if name_match else "Unnamed trainer"
        label = f"{name} (ID {trainer_id})"
        for species in sorted(species_in_text(party, {})):
            result[species].append(label)
    for species in result:
        result[species] = sorted(set(result[species]), key=lambda item: int(re.search(r"ID (\d+)", item).group(1)))
    return result


def apply_statuses(records: dict[str, SpeciesRecord], overrides: dict) -> None:
    unimplemented_abilities = overrides.get("unimplementedAbilities", {})
    species_limitations = overrides.get("speciesLimitations", {})
    for record in records.values():
        blockers = []
        limitations = []
        audits = []
        if not record.sprite_present:
            blockers.append("No matching battle-sprite source directory was found.")
        bad_abilities = [ability for ability in record.abilities if ability in unimplemented_abilities]
        for ability in bad_abilities:
            detail = unimplemented_abilities[ability]
            limitations.append(
                f"Ability {humanize_constant(ability, ('ABILITY_',))}: "
                f"{detail['reason']} ({detail['source']})"
            )
        if bad_abilities and len(bad_abilities) == len(record.abilities):
            blockers.extend(limitations)
            limitations = []
        elif bad_abilities:
            limitations.append("Use a verified ability slot when placing this Pokémon.")
        if record.hidden_ability in unimplemented_abilities:
            detail = unimplemented_abilities[record.hidden_ability]
            limitations.append(
                f"Hidden ability {humanize_constant(record.hidden_ability, ('ABILITY_',))}: "
                f"{detail['reason']} ({detail['source']})"
            )
        if record.unimplemented_level_moves:
            limitations.append(
                "One or more level-up moves are explicitly unimplemented; "
                "see the move-source column."
            )
        if not record.learnset_present:
            audits.append(
                "No explicit learnset row; confirm that this form correctly "
                "inherits a base-species learnset."
            )
        if record.constant in species_limitations:
            detail = species_limitations[record.constant]
            limitations.append(f"{detail['reason']} ({detail['source']})")
        if blockers:
            record.status = "Do not add"
            record.reasons = blockers + limitations + audits
        elif limitations:
            record.status = "Usable with limitations"
            record.reasons = limitations + audits
        elif audits:
            record.status = "Needs audit"
            record.reasons = audits
        else:
            record.status = "Ready"
            record.reasons = ["No known source-level blocker."]


def generation_for(number: int) -> str:
    boundaries = (
        (151, "Generation I"),
        (251, "Generation II"),
        (386, "Generation III"),
        (493, "Generation IV"),
        (649, "Generation V"),
        (721, "Generation VI"),
        (809, "Generation VII"),
        (905, "Generation VIII"),
        (1025, "Generation IX"),
    )
    for maximum, label in boundaries:
        if number <= maximum:
            return label
    return "Additional canonical entries"


def compact_list(values: list[str] | set[str]) -> str:
    return "<br>".join(markdown_escape(value) for value in sorted(values)) if values else "—"


def render_move_sources(record: SpeciesRecord) -> str:
    categories = (
        ("Level-up (species limitation)", record.unimplemented_level_moves),
        ("Machine/TM (does not restrict species)", record.unimplemented_machine_moves),
        ("Tutor (does not restrict species)", record.unimplemented_tutor_moves),
        ("Egg move (does not restrict species)", record.unimplemented_egg_moves),
    )
    details = []
    for label, moves in categories:
        if moves:
            details.append(f"**{label}:** {', '.join(markdown_escape(move) for move in moves)}")
    return "<br>".join(details) if details else "—"


def render(records: dict[str, SpeciesRecord], locations: dict[str, set[str]], trainers: dict[str, list[str]]) -> str:
    counts = defaultdict(int)
    used_count = catchable_count = trainer_count = 0
    for record in records.values():
        counts[record.status] += 1
        catchable = bool(locations.get(record.constant))
        trained = bool(trainers.get(record.constant))
        catchable_count += catchable
        trainer_count += trained
        used_count += catchable or trained
    engine_ids, national_dex = parse_species_numbers()
    reserved_count = len(engine_ids) - len(national_dex)

    lines = [
        "# Pokémon implementation and content availability",
        "",
        "This file is generated by `tools/generate_pokemon_availability.py`. Do not edit its tables by hand.",
        "",
        "## How to read this tracker",
        "",
        "- **Ready**: the repository contains species data and battle-sprite source, an explicit learnset is present, and this audit found no known core ability, level-up move, or evolution limitation. Unimplemented TM, tutor, and Egg moves do not restrict the species because they are optional.",
        "- **Usable with limitations**: the Pokémon can be placed deliberately, but the listed ability, move, or evolution behavior is incomplete. Review and control the affected slot or content before adding it.",
        "- **Needs audit**: source data exists, but a form-specific detail such as learnset inheritance cannot be proven by this static audit.",
        "- **Do not add**: a required core asset or every configured ordinary ability is known to be unavailable.",
        "- **Used** means present in at least one generated wild-encounter table or trainer party. **Catchable** lists ordinary, swarm, Safari Zone, and Headbutt tables whose method is enabled and obtainable in this project.",
        "",
        "> **Dex number and engine ID are different after Arceus.** HGSS reserves engine IDs 494–543 for Egg/Bad Egg and numbered placeholder records. Victini is National Dex 494 but engine ID 544; later canonical Pokémon retain that 50-ID offset. The reserved records are not Pokémon and are excluded from the tables below.",
        "",
        "> Scripted gifts, starters, trades, fossils, roaming Pokémon, and one-off static encounters do not share one authoritative table and are not yet included in the automated Used/Catchable columns. Absence from those columns therefore means “not in a tracked wild table or trainer party,” not necessarily “impossible to obtain by every route.”",
        "",
        "> This is a source audit, not runtime verification. “No known source-level blocker” must not be described as proof that every interaction, animation, form change, evolution, move, or ability works in-game.",
        "",
        "## Authoritative inputs",
        "",
        "- Species records and ordinary abilities: `data/Species.c`",
        "- Hidden abilities: `data/HiddenAbilityTable.c`",
        "- Learnsets: `data/learnsets/learnsets.json`",
        "- Explicit move implementation flags: `data/Moves.c`",
        "- Ordinary, Safari Zone, and Headbutt encounters: `data/Encounters.c`, `data/SafariEncounters.c`, and `data/Headbutt.c`",
        "- Trainer names, IDs, and parties: `data/Trainers.c`",
        "- Source-proven limitations and project-wide unavailable encounter methods: `data/pokemon_availability_overrides.json`",
        "",
        "## Summary",
        "",
        "| Metric | Count |",
        "| --- | ---: |",
        f"| Species/form records audited | {len(records)} |",
        f"| Canonical Pokémon | {len(national_dex)} |",
        f"| Alternate forms | {len(records) - len(national_dex)} |",
        f"| Reserved non-Pokémon engine records excluded | {reserved_count} |",
        f"| Ready | {counts['Ready']} |",
        f"| Usable with limitations | {counts['Usable with limitations']} |",
        f"| Needs audit | {counts['Needs audit']} |",
        f"| Do not add | {counts['Do not add']} |",
        f"| Used in tracked content | {used_count} |",
        f"| Present in enabled wild tables | {catchable_count} |",
        f"| Used by at least one trainer | {trainer_count} |",
        "",
        "## Canonical species",
        "",
    ]

    canonical_groups: dict[str, list[SpeciesRecord]] = defaultdict(list)
    forms = []
    for record in records.values():
        if record.dex_number is None:
            forms.append(record)
        else:
            canonical_groups[generation_for(record.dex_number)].append(record)

    for generation, group in canonical_groups.items():
        group.sort(key=lambda item: item.dex_number or 0)
        lines.extend([
            f"<details><summary>{generation} ({len(group)} species)</summary>",
            "",
            "| Dex | Engine ID | Pokémon | Status | Known blockers or limitations | Unimplemented moves by source | Used | Catchable locations | Trainers |",
            "| ---: | ---: | --- | --- | --- | --- | :---: | --- | --- |",
        ])
        for record in group:
            wild = locations.get(record.constant, set())
            trainer_list = trainers.get(record.constant, [])
            used = "Yes" if wild or trainer_list else "No"
            lines.append(
                f"| {record.dex_number} | {record.engine_id} | "
                f"{markdown_escape(record.name)} | "
                f"{record.status} | {compact_list(record.reasons)} | "
                f"{render_move_sources(record)} | {used} | "
                f"{compact_list(wild)} | {compact_list(trainer_list)} |"
            )
        lines.extend(["", "</details>", ""])

    forms.sort(key=lambda item: (item.name, item.constant))
    lines.extend([
        "## Alternate forms",
        "",
        "Forms are listed separately because they do not have independent National Pokédex numbers. A missing explicit learnset is reported as an audit requirement rather than assumed to be broken; many forms intentionally inherit their base species' learnset.",
        "",
        "<details><summary>Alternate-form records</summary>",
        "",
        "| Pokémon/form | Constant | Status | Known blockers or limitations | Unimplemented moves by source | Used | Catchable locations | Trainers |",
        "| --- | --- | --- | --- | --- | :---: | --- | --- |",
    ])
    for record in forms:
        wild = locations.get(record.constant, set())
        trainer_list = trainers.get(record.constant, [])
        used = "Yes" if wild or trainer_list else "No"
        lines.append(
            f"| {markdown_escape(record.name)} | `{record.constant}` | "
            f"{record.status} | {compact_list(record.reasons)} | "
            f"{render_move_sources(record)} | {used} | "
            f"{compact_list(wild)} | {compact_list(trainer_list)} |"
        )
    lines.extend(
        [
            "",
            "</details>",
            "",
            "## Regeneration",
            "",
            "After changing species data, move implementation flags, learnsets, "
            "abilities, wild encounters, Headbutt/Safari encounters, or trainer "
            "parties, run:",
            "",
            "```sh",
            "python tools/generate_pokemon_availability.py",
            "```",
            "",
            "To check whether the committed tracker is current without rewriting it:",
            "",
            "```sh",
            "python tools/generate_pokemon_availability.py --check",
            "```",
            "",
        ]
    )
    return "\n".join(lines)


def parse_tm_assignments() -> list[tuple[str, str]]:
    return re.findall(
        r"^\s*(MOVE_[A-Z0-9_]+)\s*,\s*//\s*(TM\d+(?:_SV)?)\s*$",
        read("src/item.c"),
        re.MULTILINE,
    )


def render_unusable_tm_section(
    move_names: dict[str, str],
    unimplemented_moves: set[str],
) -> tuple[str, int]:
    rows = []
    for move, tm_item in parse_tm_assignments():
        if move in unimplemented_moves:
            rows.append(
                f"| {tm_item} | {markdown_escape(move_names[move])} | "
                "Unimplemented | Unusable | Do not assign | N/A |"
            )
    section = "\n".join(
        [
            TM_BEGIN_MARKER,
            "| TM item | Move | Engine status | Availability | Reward source | Verification |",
            "| --- | --- | --- | --- | --- | --- |",
            *rows,
            TM_END_MARKER,
        ]
    )
    return section, len(rows)


def render_tm_tracker(move_names: dict[str, str], unimplemented_moves: set[str]) -> str:
    current = TM_OUTPUT.read_text(encoding="utf-8")
    section, count = render_unusable_tm_section(move_names, unimplemented_moves)
    candidate_section = current.split("## Unusable TM assignments", 1)[0]
    unusable_tm_items = {
        tm_item
        for move, tm_item in parse_tm_assignments()
        if move in unimplemented_moves
    }
    invalid_candidates = [
        tm_item
        for tm_item in sorted(unusable_tm_items)
        if re.search(rf"^\| {re.escape(tm_item)} \|", candidate_section, re.MULTILINE)
    ]
    if invalid_candidates:
        raise ValueError(
            "Unimplemented TM mappings appear in the candidate table: "
            + ", ".join(invalid_candidates)
        )
    marker_pattern = re.compile(
        rf"{re.escape(TM_BEGIN_MARKER)}.*?{re.escape(TM_END_MARKER)}",
        re.DOTALL,
    )
    if not marker_pattern.search(current):
        raise ValueError(f"Generated TM markers are missing from {TM_OUTPUT.relative_to(ROOT)}")
    updated = marker_pattern.sub(section, current)
    updated, replacements = re.subn(
        r"\| Unusable TM mappings \| \d+ \|",
        f"| Unusable TM mappings | {count} |",
        updated,
    )
    if replacements != 1:
        raise ValueError("TM tracker must contain exactly one unusable-mapping summary row")
    return updated


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the generated tracker is stale")
    args = parser.parse_args()

    records = parse_species()
    move_names, unimplemented_moves = parse_moves()
    tm_items_by_move: dict[str, list[str]] = defaultdict(list)
    for move, tm_item in parse_tm_assignments():
        tm_items_by_move[move].append(tm_item)
    attach_learnset_findings(
        records,
        move_names,
        unimplemented_moves,
        tm_items_by_move,
    )
    overrides = json.loads(read("data/pokemon_availability_overrides.json"))
    apply_statuses(records, overrides)
    disabled_methods = set(overrides.get("disabledEncounterMethods", []))
    locations = merge_locations(
        parse_ordinary_encounters(disabled_methods),
        parse_safari_encounters(disabled_methods),
        parse_headbutt_encounters(),
    )
    trainers = parse_trainers()
    unknown_placements = (set(locations) | set(trainers)) - set(records)
    if unknown_placements:
        unknown = ", ".join(sorted(unknown_placements))
        raise ValueError(f"Content references species without Species.c records: {unknown}")
    canonical_numbers = {record.dex_number for record in records.values() if record.dex_number is not None}
    expected_canonical = set(range(1, max(canonical_numbers) + 1))
    if canonical_numbers != expected_canonical:
        missing = ", ".join(str(number) for number in sorted(expected_canonical - canonical_numbers))
        raise ValueError(f"Canonical species audit is incomplete; missing National Dex entries: {missing}")
    generated = render(records, locations, trainers)
    generated_tm_tracker = render_tm_tracker(move_names, unimplemented_moves)

    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        current_tm_tracker = TM_OUTPUT.read_text(encoding="utf-8")
        stale = False
        if current != generated:
            print(f"{OUTPUT.relative_to(ROOT)} is stale; regenerate it.", file=sys.stderr)
            stale = True
        if current_tm_tracker != generated_tm_tracker:
            print(f"{TM_OUTPUT.relative_to(ROOT)} is stale; regenerate it.", file=sys.stderr)
            stale = True
        if stale:
            return 1
        print(f"{OUTPUT.relative_to(ROOT)} is current.")
        print(f"{TM_OUTPUT.relative_to(ROOT)} is current.")
        return 0

    OUTPUT.write_text(generated, encoding="utf-8", newline="\n")
    TM_OUTPUT.write_text(generated_tm_tracker, encoding="utf-8", newline="\n")
    print(f"Wrote {OUTPUT.relative_to(ROOT)} with {len(records)} species/form records.")
    print(f"Updated unusable TM mappings in {TM_OUTPUT.relative_to(ROOT)}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
