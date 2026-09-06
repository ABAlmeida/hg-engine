# Expose and shorten a scripted tutorial without running its battle

## Goal

Keep the complete Route 29 counterpart scene editable in source while
retaining the Slakoth grass demonstration and skipping HGSS's simulated
capture battle and duplicate Poké Ball gift.

## Target

- Source: `armips/scr_seq/scr_seq_revised_opening.s`
- Script archive member: 225 (`scr_seq_0225_R29`)
- Script-table entry: entry 1 at `0x4`
- Original script offset: `0x1B2`
- Trigger: Route 29 event `_EV_scr_seq_R29_001 + 1`

The patch validates the original table entry and opening commands, redirects
entry 1 to `route_29_catching_tutorial`, and appends an editable copy of the
scene and all of its movement blocks. Do not edit `build/a012/2_225` or the
reference checkout under `.scratch/`; neither is the permanent source.

## Verified HGSS commands

The control flow and command parameters come from
`.scratch/pret-pokeheartgold/files/fielddata/script/scr_seq/scr_seq_0225_R29.s`.
That HGSS source verifies the uses of `ScrCmd_609`, `LockAll`, `PlayCry`,
`WaitCry`, `ApplyMovement`, `WaitMovement`, `CallStd`, `GenderMsgBox`,
`CloseMsg`, `GetPlayerCoords`, `Release`, `Compare`, conditional branches,
`Lock`, `BufferPlayersName`, follower commands 602-604, `Wait`, `HidePerson`,
`SetFlag`, `SetVar`, `ReleaseAll`, and `End`.

The original opcode 251 `CatchingTutorial` at offset `0x4FB` is deliberately
absent from the expanded copy. The subsequent Poké Ball gift and obsolete
post-battle messages are also absent.

Movement steps use `step TYPE, REPEAT_COUNT` and end with `step_end`. Numeric
types are verified against the HGSS movement macros in
`.scratch/pret-pokeheartgold/asm/macros/movement.inc` and their constants in
`.scratch/pret-pokeheartgold/include/constants/movements.h`. The expanded
source gives those numbers descriptive `MOVE_*` aliases.

## Verified identifiers

- Map: Route 29 (`MAP_ROUTE_29` / map 33).
- Event data:
  `.scratch/pret-pokeheartgold/files/fielddata/eventdata/zone_event/030_R29.json`.
- Counterpart: object 6, originally `obj_R29_var_2`, exposed as
  `ROUTE_29_FRIEND_OBJECT_ID`.
- Slakoth: object 7, originally
  `obj_R29_tsure_poke_static_marill`, exposed as
  `ROUTE_29_MARILL_OBJECT_ID`. The legacy object name remains because the
  Heartless Gold sprite/species replacement does not renumber the map event.
- Player: object 255 (`PLAYER_OBJECT_ID`).
- Coordinate variables: `VAR_TEMP_x4000` and `VAR_TEMP_x4001`.
- Messages: local 17/18 for the initial greeting, 0/1 for the Bait explanation,
  and 21/22 for the gender-selected counterpart's closing response. Their
  source is `data/text/373.txt`. Local message 2 belongs to the superseded
  Lyra-only helper and is no longer called by the expanded scene.
- Completion state: `FLAG_HIDE_ROUTE_29_FRIEND`,
  `FLAG_HIDE_ROUTE_29_MARILL`, `VAR_UNK_408B`, and `FLAG_UNK_09A`, copied from
  the original scene cleanup.
- Trainers and items: not used. The original five-Ball reward is intentionally
  removed.

## Editable pattern

```asm
.org ROUTE_29_TUTORIAL_TABLE_ENTRY
.word route_29_catching_tutorial - (ROUTE_29_TUTORIAL_TABLE_ENTRY + 4)

.org ROUTE_29_TUTORIAL_APPEND_OFFSET
route_29_catching_tutorial:
lockall
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_counterpart_reacts
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_jumps_into_grass
wait_movement
releaseall
end

.align 4
route_29_slakoth_jumps_into_grass:
step MOVE_DELAY_8, 3
step MOVE_JUMP_FAR_WEST, 1
step_end
```

Keep each movement label aligned, retain `step_end`, and update both actors'
paths together when changing their final coordinates. The complete source has
separate labels for arrival, seven possible approach rows, seven staging rows,
a single shared player-staging path, the shared counterpart demonstration,
and departure. It deliberately does not return the actors to their pre-demo
positions.

## Control-flow checklist

- All seven trigger rows converge at `route_29_approach_done` and
  `route_29_stage_done`.
- Lyra and Ethan share the same movement calls; only `gender_msgbox` selects
  their presentation-specific dialogue.
- Every group of simultaneous `apply_movement` commands reaches a matching
  `wait_movement` before dependent dialogue or cleanup.
- Follower movement commands 602-604 retain their original ordering.
- The counterpart's gender-selected closing message remains visible until
  input before `closemsg`.
- After crossing the grass, Slakoth starts one tile west of the counterpart.
  Its departure begins by entering the counterpart's vacated tile, repeats the
  same turns, and omits the leader's final step so it finishes one tile behind.
  This is deterministic scripted movement, not an arbitrary-overworld follow
  command.
- `lockall` reaches exactly one `releaseall` on every path.
- Both objects are hidden and both one-time hide flags are set before `end`.
- No path invokes `catching_tutorial`, gives Poké Balls, or starts a battle
  task.

## Build and manual verification

Run `make quick-rom -j$(nproc)` from MSYS2 UCRT64. The expected ROM output is
`test.nds`; a successful build does not establish that the choreography is
correct.

Manually check both protagonist genders and all accessible north/south trigger
rows. Confirm that no actor collides, walks through blocked tiles, or finishes
off-screen; the Slakoth demonstration and messages complete; following-Pokémon
movement resumes; both NPCs depart and stay hidden after re-entry; and no
tutorial battle, Ball gift, item use, Pokédex update, or capture-rule mutation
occurs.
