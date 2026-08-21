# Make a static encounter permanently available

## Goal

Make a time-gated static encounter available every day until the first battle
finishes, then permanently remove it for every battle outcome.

## Target

- Script archive/source: script NARC member 58, reproduced by
  `armips/scr_seq/scr_seq_00058_union_cave_lapras.s`
- Script entries: Union Cave B2F Lapras battle, OnLoad visibility, and
  OnTransition visibility
- Trigger or caller: map `D25R0103`; the event header assigns object 3 to
  Lapras, and header member 319 calls entry 1 OnLoad and entry 2 OnTransition

## Verified HGSS commands

- `goto_if_set FLAG, LABEL`, `setflag FLAG`, and `clearflag FLAG` copy their
  argument ordering from existing HeartGold scripts and the definitions in
  `armips/include/scriptmacros.s`.
- `play_cry SPECIES, 0` and `wait_cry` preserve the original member 58 command
  sequence and parameter ordering.
- `wild_battle SPECIES, LEVEL, SHINY` uses species, level, and shiny parameters
  in that order, verified by the original member 58 and its macro definition.
- `check_battle_won VAR` stores whether the player lost. The original Lapras
  script compares its result with zero before calling `white_out`.
- `hide_person OBJECT_ID` preserves the original member 58 OnLoad behavior.

## Verified identifiers

- `FLAG_UNION_CAVE_LAPRAS_CONSUMED` is flag `0x407`/1031. Before allocation,
  `armips/include/flags.s` named it `FLAG_UNK_407` inside the spare hidden-item
  flag range, and a repository-wide search found no use. It is below
  `DAILY_FLAG_BASE` (`0xAA0`/2720), so the daily reset does not clear it.
- `FLAG_HIDE_UNION_CAVE_LAPRAS` and `FLAG_ENGAGING_STATIC_POKEMON` are the two
  flags used by the original member 58.
- `SPECIES_LAPRAS` and level 20 are preserved from the original member 58.
- Map `D25R0103` and Lapras object ID 3 are verified in
  `.scratch/pokeheartgold-reference/files/fielddata/script/scr_seq/event_D25R0103.h`.
- Messages, trainers, items, and other content IDs: Not used.

## Minimal pattern

```asm
encounter_visibility:
    goto_if_set FLAG_ENCOUNTER_CONSUMED, hide_encounter
    clearflag FLAG_HIDE_ENCOUNTER
    end

hide_encounter:
    setflag FLAG_HIDE_ENCOUNTER
    end

encounter_battle:
    setflag FLAG_ENGAGING_STATIC_POKEMON
    wild_battle SPECIES, LEVEL, 0
    clearflag FLAG_ENGAGING_STATIC_POKEMON
    setflag FLAG_ENCOUNTER_CONSUMED
    check_battle_won VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 0
    goto_if_eq encounter_blackout
    releaseall
    end

encounter_blackout:
    white_out
    releaseall
    end
```

Set the consumed flag immediately after `wild_battle` returns and before the
loss branch when catching, defeating, fleeing, and blacking out must all consume
the encounter.

## Control-flow checklist

- Preserve the original script table order expected by the map header.
- Keep `FLAG_ENGAGING_STATIC_POKEMON` around the battle so the active object is
  hidden while the field reloads.
- Set the permanent flag before branching to `white_out` if losses consume the
  encounter.
- Keep `lockall` and `releaseall` balanced on both non-loss and loss paths.
- Do not reuse a daily flag for an encounter that must remain consumed.
- Audit and name an explicitly unused persistent flag; never guess one.

## Build and manual verification

When explicitly requested, run:

```sh
make quick-rom -j$(nproc)
```

Using a normal in-game save rather than a save state, confirm that Lapras is
visible on a non-Friday. Check separate saves for catching, defeating, fleeing,
and blacking out. After each outcome, leave and re-enter Union Cave B2F, advance
the date, save, and reload; Lapras must remain absent.
