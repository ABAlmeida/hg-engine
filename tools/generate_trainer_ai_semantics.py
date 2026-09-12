#!/usr/bin/env python3
import argparse
import json
import re
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--effects", required=True)
    parser.add_argument("--moves", required=True)
    parser.add_argument("--abilities", required=True)
    parser.add_argument("--types", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    source = json.loads(Path(args.input).read_text(encoding="utf-8"))

    def constants(path, prefix):
        text = Path(path).read_text(encoding="utf-8")
        return {
            name: int(value)
            for name, value in re.findall(
                rf"^#define\s+({prefix}[A-Z0-9_]+)\s+(\d+)\s*$", text, re.MULTILINE
            )
        }

    effects = constants(args.effects, "MOVE_EFFECT_")
    moves = constants(args.moves, "MOVE_")
    abilities = constants(args.abilities, "ABILITY_")
    types = constants(args.types, "TYPE_")

    effect_class_ids = {
        "poison": 1,
        "toxic": 2,
        "burn": 3,
        "paralysis": 4,
        "sleep": 5,
        "delayedSleep": 6,
        "leechSeed": 7,
        "recoverHalf": 8,
        "protect": 9,
        "rest": 10,
        "trickRoom": 11,
        "attackDefenseUp": 12,
        "defenseSpecialDefenseUp": 13,
        "specialAttackSpecialDefenseUp": 14,
        "attackSpeedUp": 15,
        "flee": 16,
        "flinch": 17,
        "endure": 18,
        "curse": 19,
        "curePartyStatus": 20,
    }
    move_class_ids = {
        "tailwind": 1,
        "lightScreen": 2,
        "reflect": 3,
        "spikes": 4,
        "toxicSpikes": 5,
        "stealthRock": 6,
        "stickyWeb": 7,
        "substitute": 8,
        "randomAction": 9,
    }
    ability_type_behavior_ids = {
        "immune": 1,
        "absorbHeal": 2,
        "absorbSpecialAttack": 3,
        "absorbSpeed": 4,
        "absorbAttack": 5,
        "absorbBoost": 6,
        "multiply50": 7,
        "multiply125": 8,
    }

    def dense_classes(section, known, class_ids, label):
        result = [0] * (max(known.values()) + 1)
        for class_name, names in source.get(section, {}).items():
            if class_name not in class_ids:
                raise SystemExit(f"unknown {label} class: {class_name}")
            for name in names:
                if name not in known:
                    raise SystemExit(f"unknown {label}: {name}")
                index = known[name]
                if result[index] != 0:
                    raise SystemExit(f"duplicate {label} semantics: {name}")
                result[index] = class_ids[class_name]
        return result

    effect_classes = dense_classes("effectClasses", effects, effect_class_ids, "move effect")
    effect_rows = [(effect, kind) for effect, kind in enumerate(effect_classes) if kind]
    if len(effect_classes) > 512 or max(effect_class_ids.values()) > 127:
        raise SystemExit("packed effect semantics exceed the u16 encoding")
    move_rows = []
    seen_moves = set()
    for class_name, names in source.get("moveClasses", {}).items():
        if class_name not in move_class_ids:
            raise SystemExit(f"unknown move class: {class_name}")
        for name in names:
            if name not in moves:
                raise SystemExit(f"unknown move: {name}")
            if moves[name] in seen_moves:
                raise SystemExit(f"duplicate move semantics: {name}")
            seen_moves.add(moves[name])
            move_rows.append((moves[name], move_class_ids[class_name]))

    ability_type_rows = []
    for rule in source.get("abilityTypeRules", []):
        try:
            row = (
                abilities[rule["ability"]],
                types[rule["type"]],
                ability_type_behavior_ids[rule["behavior"]],
            )
        except KeyError as exc:
            raise SystemExit(f"unknown ability type rule value: {exc.args[0]}") from exc
        if any(existing[:2] == row[:2] for existing in ability_type_rows):
            raise SystemExit(f"duplicate ability type rule: {rule}")
        ability_type_rows.append(row)

    transition_ids = {
        "forceRepeatScaling": 1,
        "repeatScaling": 2,
    }
    transitions = []
    transition_moves = set()
    for class_name, names in source.get("moveTransitions", {}).items():
        if class_name not in transition_ids:
            raise SystemExit(f"unknown move transition: {class_name}")
        for name in names:
            if name not in moves:
                raise SystemExit(f"unknown move: {name}")
            if moves[name] in transition_moves:
                raise SystemExit(f"duplicate move transition: {name}")
            transition_moves.add(moves[name])
            maximum_progress = 5 if class_name == "forceRepeatScaling" else 3
            transitions.append((moves[name], transition_ids[class_name], maximum_progress))
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    transitions = sorted(set(transitions))
    if any(move > 0x3FF or kind > 0x3F for move, kind in move_rows):
        raise SystemExit("packed move classes exceed the u16 encoding")
    if any(ability > 0xFF or type_id > 0x1F or behavior < 1 or behavior > 8
           for ability, type_id, behavior in ability_type_rows):
        raise SystemExit("packed ability/type semantics exceed the u16 encoding")
    if any(move > 0x3FF or kind > 3 or progress > 0xF
           for move, kind, progress in transitions):
        raise SystemExit("packed move transitions exceed the u16 encoding")

    def enum_lines(prefix, values):
        rows = [f"#define {prefix}_NONE 0"]
        rows.extend(f"#define {prefix}_{name.upper()} {value}" for name, value in values.items())
        return "\n".join(rows)

    def array(name, values):
        return f"static const u16 {name}[{len(values)}] = {{ " + ", ".join(map(str, values)) + " };"

    sections = [
        "#ifndef TRAINER_AI_SEMANTICS_GENERATED_H",
        "#define TRAINER_AI_SEMANTICS_GENERATED_H",
        "",
        enum_lines("AI_EFFECT_CLASS", effect_class_ids),
        "#define AI_EFFECT_ID_MASK 0x1FF",
        "#define AI_EFFECT_CLASS_SHIFT 9",
        f"#define AI_EFFECT_ID_LIMIT {len(effect_classes)}",
        f"#define AI_EFFECT_CLASS_ENTRY_COUNT {len(effect_rows)}",
        array("sAIEffectClassEntries",
              [effect | (kind << 9) for effect, kind in effect_rows]),
        "",
        enum_lines("AI_MOVE_CLASS", move_class_ids),
        "#define AI_MOVE_CLASS_ID_MASK 0x3FF",
        "#define AI_MOVE_CLASS_SHIFT 10",
        f"#define AI_MOVE_CLASS_ENTRY_COUNT {len(move_rows)}",
        array("sAIMoveClasses", [move | (kind << 10) for move, kind in sorted(move_rows)]),
        "",
        enum_lines("AI_ABILITY_TYPE", ability_type_behavior_ids),
        "#define AI_ABILITY_ID_MASK 0xFF",
        "#define AI_ABILITY_TYPE_SHIFT 8",
        "#define AI_ABILITY_TYPE_MASK 0x1F",
        "#define AI_ABILITY_TYPE_BEHAVIOR_SHIFT 13",
        f"#define AI_ABILITY_TYPE_RULE_COUNT {len(ability_type_rows)}",
        array("sAIAbilityTypeRules", [ability | (type_id << 8) | ((behavior - 1) << 13)
                                      for ability, type_id, behavior in sorted(ability_type_rows)]),
        "",
        "#define AI_MOVE_TRANSITION_NONE 0",
        "#define AI_MOVE_TRANSITION_FORCE_REPEAT_SCALING 1",
        "#define AI_MOVE_TRANSITION_REPEAT_SCALING 2",
        "#define AI_MOVE_TRANSITION_ID_MASK 0x3FF",
        "#define AI_MOVE_TRANSITION_KIND_SHIFT 10",
        "#define AI_MOVE_TRANSITION_KIND_MASK 0x3",
        "#define AI_MOVE_TRANSITION_PROGRESS_SHIFT 12",
        f"#define AI_MOVE_TRANSITION_COUNT {len(transitions)}",
        array("sAIMoveTransitions", [move | (kind << 10) | (progress << 12)
                                      for move, kind, progress in transitions]),
        "",
        "#endif",
        "",
    ]
    output.write_text("\n".join(sections), encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
