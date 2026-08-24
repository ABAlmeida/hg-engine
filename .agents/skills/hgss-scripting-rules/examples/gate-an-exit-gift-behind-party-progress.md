# Gate an exit gift behind party progress

## Goal

Keep a coordinate-triggered NPC exit gate active until a tracked story event
is complete and the relevant party Pokémon has reached a required level.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_00843_elm_healing_kit.s`
- Script entry: member 843, entry 3
- Trigger: Elm's assistant coordinate event across `(x=3..6, z=11)` while
  `VAR_SCENE_ELMS_LAB` is 1

## Verified HGSS commands

- `goto_if_unset FLAG_SHOWED_TOGEPI_TO_ELM, target` uses the repository's
  established `checkflag` plus conditional-branch wrapper. The flag is set by
  member 843 only after Elm accepts the hatched Togepi line and completes the
  existing Eviolite gift flow.
- `get_party_slot_with_species destination, species` is HGSS command 647.
  Its `ScrCmd_GetPartySlotWithSpecies` handler was verified in
  pret/pokeheartgold `src/scrcmd_party.c`: it searches non-Egg party members
  and returns the slot, or 255 when absent.
- `mon_get_level destination, party_slot` is HGSS command 535. Its
  `ScrCmd_MonGetLevel` handler was verified in the same HGSS source: it writes
  zero for an Egg and otherwise writes the selected party member's level.
- `apply_movement PLAYER_OBJECT_ID, 0x21C` reuses member 843's original
  one-step player retreat from the lab's pre-starter escape blocker.
- The assistant approach and departure movements are the original member-843
  sequences already used by the replacement gift flow.

## Verified identifiers

- Flag: `FLAG_SHOWED_TOGEPI_TO_ELM` is flag 114, formerly named
  `FLAG_UNK_072`; member 843 sets it in the successful hatch-show branch.
- Variables: `VAR_TEMP_x4000` retains the triggering x-coordinate;
  `VAR_TEMP_x4002` and `VAR_TEMP_x4003` hold the found party slot and
  species/level respectively.
- Messages: local IDs 110 and 111 in `data/text/543.txt` explain the hatch/show
  and level-5 requirements.
- Map/event: Elm's lab event bank 58 identifies assistant object 2 and the
  entry-3 coordinate event.
- Species: Togepi, Togetic and Togekiss are the same three species accepted by
  Elm's original hatch-show branch.
- Trainer IDs and item IDs: no new identifiers. The existing gift sequence
  retains its four verified item IDs.

## Minimal pattern

```asm
goto_if_unset FLAG_SHOWED_TOGEPI_TO_ELM, needs_hatch
call find_story_mon
compare VAR_TEMP_x4002, 255
goto_if_eq needs_hatch
mon_get_level VAR_TEMP_x4003, VAR_TEMP_x4002
compare VAR_TEMP_x4003, 5
goto_if_lt needs_level
goto existing_gift

needs_hatch:
npc_msg HATCH_MESSAGE
goto turn_back

needs_level:
npc_msg LEVEL_MESSAGE

turn_back:
wait_button
closemsg
apply_movement PLAYER_OBJECT_ID, VERIFIED_RETREAT_MOVEMENT
goto existing_departure
```

## Control-flow checklist

- Reject a missing party species before calling `mon_get_level`; slot 255 is
  not a valid party index.
- Preserve the trigger's x-coordinate when temporary variables are reused for
  the party lookup, because the departure movement depends on it.
- Move the player off the coordinate trigger before releasing control so the
  next exit attempt triggers the gate again.
- Advance the scene only after every prerequisite and every atomic gift-space
  check succeeds.
- Make rejection dialogue wait for input and close before movement.
- If the related story interaction only checks the party lead, redirect that
  lookup to the same party-wide helper instead of imposing an undocumented
  party-order requirement.

## Build and manual verification

When explicitly requested, run `make quick-rom -j$(nproc)`. On a new save,
check unhatched rejection, hatched-but-unshown rejection, recognition from a
non-lead slot, below-level-5 rejection, success at levels 5 and above, repeat
attempts, full-Bag retry without partial gifts, save/reload at each state, and
normal post-exit Silver progression. A successful build does not prove the
script flow.
