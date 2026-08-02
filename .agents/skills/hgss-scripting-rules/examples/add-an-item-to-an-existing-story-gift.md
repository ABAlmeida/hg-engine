# Add an item to an existing HGSS story gift

## Goal

Give the Old Rod during Professor Oak's existing Mr. Pokémon sequence without
replacing Oak's departure or adding a new save field.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_revised_opening.s`
- Script member: 229, Oak continuation at `0x2E8`
- Trigger: the existing Mr. Pokémon house story scene

## Verified HGSS commands and identifiers

- `setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD` and
  `setvar VAR_SPECIAL_x8005, 1` use the variables consumed by
  `callstd std_obtain_item_verbose`; the same ordering is used by the Elm and
  Violet gifts in this repository.
- `setflag FLAG_GOT_OLD_ROD` uses the vanilla ownership flag. Its original gift
  is member 233 (`scr_seq_0233_R32PC0101.s`).
- Oak is object 1 in `event_R30R0201.h`.
- The displaced Oak departure is `apply_movement 1, 0x450`, followed by the
  original continuation at `0x359`.
- Message 33 is appended to `data/text/377.txt`; it is not an invented existing
  message ID.

## Minimal pattern

```asm
npc_msg NEW_MESSAGE
setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setflag FLAG_GOT_OLD_ROD
closemsg
apply_movement OAK_OBJECT_ID, ORIGINAL_MOVEMENT
goto ORIGINAL_CONTINUATION
```

The early-game Key Items pocket cannot be full in this sequence, so the gift
does not add a retry branch that could strand the player inside a locked story
cutscene. The vanilla ownership flag prevents the later fisherman gift.

## Control-flow and manual verification

The original script owns `lockall` and its final `releaseall`; this routine
neither acquires nor releases the lock. It replays the overwritten departure
movement before returning to the original wait and cleanup.

When explicitly requested, build with `make quick-rom -j$(nproc)`. On a new
save, verify Oak gives exactly one Old Rod, leaves normally, Route 30 remains
open, and the Route 32 fisherman recognizes `FLAG_GOT_OLD_ROD` and does not
give a second copy.
