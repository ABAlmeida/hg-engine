# Give a configured one-time Egg

## Goal

Give one Egg selected by a map-local interaction while sharing the party-space
check, Egg creation, fixed-IV/ability configuration, and completion-flag update
with future NPC gifts.

## Target

- Common script: `armips/scr_seq/scr_seq_00003_commonscript.s`, standard
  script 2075 (`std_configured_egg_gift`).
- Example caller: `armips/scr_seq/scr_seq_00864_violet_egg_gift.s`, member
  864, entry 1.
- Trigger: object 1 (`obj_T22R0601_gsboy1`) in Violet's trade house, verified
  in pret zone event 156 (`T22R0601`) at coordinates `(2, 5)`.

## Verified HGSS commands

- `get_party_count VAR_SPECIAL_RESULT`: command 332; copied from the Primo Egg
  gifts in vanilla member 860, where count 6 refuses the gift.
- `give_egg species, location`: command 138; parameter 1 is read through
  `ScriptGetVar` as the species and parameter 2 as the gift-location offset in
  `src/field/script_commands.c`. Vanilla member 860 supplies species followed
  by `MAPLOC(METLOC_PRIMO)` in this order.
- `setflagvar VAR_SPECIAL_x8006`: command 33; its variable-based flag form is
  used by vanilla members 145, 151, and 952. It runs only after `give_egg`.
- `callstd std_configured_egg_gift`: command 20; the common-script pattern and
  `endstd` termination match existing standard scripts in member 3.
- `ConfigureGiftEgg VAR_SPECIAL_x8007`: the established command-208 custom
  dispatcher invokes project subcommand 5. The variable contains a packed
  5-bit IV value and 9-bit ability ID; the implementation configures the last
  party member immediately after the verified `give_egg` call.
- `menu_init`, `menu_item_add`, and `menu_exec`: commands 750-752; parameter
  ordering is copied from existing member 3 menus. The first two menu-item
  parameters are local message IDs and the third is the result value.
- `play_fanfare SEQ_ME_TAMAGO_GET`, `wait_fanfare`: copied from the successful
  Primo Egg receipt path in vanilla member 860.

## Verified identifiers

- Inputs: `VAR_SPECIAL_x8004` species, `VAR_SPECIAL_x8005` gift-location
  offset, `VAR_SPECIAL_x8006` completion flag, and `VAR_SPECIAL_x8007` packed
  IV/ability configuration. Output:
  `VAR_SPECIAL_RESULT`, with 1 for success and 0 for a full party.
- Violet completion flag: `FLAG_TRADE_VIOLET_CITY_BELLSPROUT_ONIX` (113), the
  original one-time trade flag in member 864 and `armips/include/flags.s`.
- Species: `SPECIES_SANDILE` 601, `SPECIES_TINKATINK` 1007, and
  `SPECIES_SWINUB` 220 from `asm/include/species.inc`.
- Abilities: Moxie 153, Mold Breaker 104, and Thick Fat 47 from
  `asm/include/abilities.inc`. Sandile receives Moxie, Tinkatink receives Mold
  Breaker, and Swinub receives its hidden ability, Thick Fat.
- Location: `METLOC_JOHTO` is 2004 in HGSS `map_sections.h`; `GiveEgg` expects
  the verified `MAPLOC` offset, 4.
- Messages: Violet message bank 562. The replacement text file defines local
  IDs 0-9; the menu uses 2-5 and the outcome branches use 6-9.
- Event/map: member 864 and event member 156. The existing object already
  calls entry 1, so no event-data change is required.
- Trainer and item IDs: Not used.

## Minimal pattern

Caller:

```asm
setvar VAR_SPECIAL_x8004, SPECIES_SANDILE
setvar VAR_SPECIAL_x8005, METLOC_JOHTO_OFFSET
setvar VAR_SPECIAL_x8006, FLAG_ONE_TIME_GIFT
SetGiftEggConfig VAR_SPECIAL_x8007, 25, ABILITY_MOXIE
callstd std_configured_egg_gift
compare VAR_SPECIAL_RESULT, 0
goto_if_eq party_full
```

Common helper:

```asm
get_party_count VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 6
goto_if_ge party_full
give_egg VAR_SPECIAL_x8004, VAR_SPECIAL_x8005
ConfigureGiftEgg VAR_SPECIAL_x8007
setflagvar VAR_SPECIAL_x8006
setvar VAR_SPECIAL_RESULT, 1
endstd
end
```

## Control-flow checklist

- The caller owns `lockall`, messages, fanfare, `releaseall`, and termination.
- Cancel and full-party paths do not set the completion flag.
- The helper checks party space immediately before `give_egg` and sets the
  flag only after Egg configuration, keeping the one-time gift retryable.
- The custom command targets the newly added final party member. It sets all
  six IVs, records the normal/hidden ability-slot metadata, writes the selected
  ability, and recalculates party stats. It must remain directly after
  `give_egg`; inserting any party mutation between the two would configure the
  wrong Pokémon.
- All four Violet menu results reach a path that closes the message, releases
  the field, and ends.
- The helper has no map-local dialogue, so other maps can reuse it with their
  own messages, menus, species, locations, and flags.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. Verify each
type on a separate pre-gift save, Cancel followed by retry, full-party refusal
followed by retry, save/reload after receipt, correct Egg species and hatch,
the six 25 IVs, Moxie/Thick Fat/Mold Breaker assignments, ability persistence
after evolution, and the post-gift dialogue. A successful build does not prove
script flow.
