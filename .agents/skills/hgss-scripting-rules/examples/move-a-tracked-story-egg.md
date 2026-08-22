# Move a tracked HGSS story Egg gift

## Goal

Give Elm's normal Togepi Egg immediately after the starter and Healing Kit,
while preserving the identity fields used by the later hatch phone call.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_00843_elm_healing_kit.s`
- Script member: 843, appended starter-gift continuation
- Trigger: the existing successful starter and nickname path

## Verified HGSS commands

- `apply_movement ELM_OBJECT_ID, ELM_TURN_TO_PLAYER_MOVEMENT` and
  `wait_movement` replay the exact movement displaced by the existing Healing
  Kit hook. Finish both before opening the gift dialogue so Elm faces the
  player.
- `give_togepi_egg`: command 776, copied from the project's former Mr. Pokémon
  Egg replacement. Its implementation in `src/field/script_commands.c` adds
  the normal Togepi Egg and saves its personality and gender for the later Elm
  hatch event.
- `play_fanfare SEQ_ME_TAMAGO_GET` and `wait_fanfare` are copied from the
  successful Primo Egg receipt path in vanilla member 860 and the former
  Mr. Pokémon gift.
- `npc_msg` uses appended local messages from member 843's existing message
  bank. The script closes the previous verbose item message before starting
  the Egg dialogue.

## Verified identifiers

- Elm object: object 0 in member 843's existing starter sequence.
- Movement: original member-843 target `0x390`, preserved by the hook's binary
  assertion.
- Messages: local IDs 107-109 in `data/text/543.txt` introduce, announce, and
  explain the Egg.
- Species: implicit Togepi behavior owned by verified command 776; no new
  species ID is supplied.
- Flags, variables, trainer IDs, map IDs, and item IDs: No new identifiers.

## Minimal pattern

```asm
apply_movement ELM_OBJECT_ID, ELM_TURN_TO_PLAYER_MOVEMENT
wait_movement

// Existing Healing Kit verbose gift.
closemsg
npc_msg ELM_EGG_INTRO_MESSAGE
give_togepi_egg
buffer_players_name 0
npc_msg ELM_EGG_RECEIVED_MESSAGE
play_fanfare SEQ_ME_TAMAGO_GET
wait_fanfare
npc_msg ELM_EGG_EXPLANATION_MESSAGE
closemsg
```

## Control-flow checklist

- Run only after a starter has been successfully added and named.
- The normal opening has one party member here, so command 776 has space for
  the Egg; do not reuse this pattern in a context where the party can be full.
- Finish the displaced turn before either gift dialogue.
- Do not replace command 776 with generic `give_egg` unless the later Elm
  identity consumers are deliberately redesigned.
- Resume after the original `wait_movement`; otherwise the already completed
  movement is waited on twice.
- Preserve the phone registration, scene updates, release, and all later
  hatch-related state.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. On a new
save, verify Elm faces the player for both gifts, gives exactly one normal
Togepi Egg, registers his number, the assistant still triggers, the Egg hatches
normally, and Elm's hatch phone call and Eviolite interaction still work.
A successful build does not prove script flow.
