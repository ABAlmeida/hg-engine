# Disable automatic trainer line of sight

## Goal

Require the player to talk to ordinary field trainers while leaving their
interaction behavior under the shared trainer script's control.

## Target

- Detection function: US HeartGold `TryGetSeenByNpcTrainers` at `0x020640C8`
- Automatic trigger: standard trainer-approach script
- Talk trigger: the interacted map object's existing trainer script

## Verified control flow

- `FieldInput_Process` calls `TryGetSeenByNpcTrainers` before processing
  movement and direct interaction.
- The native function starts `std_trainer_approach` and records the engaging
  trainer after a successful sight check.
- The separate interaction branch starts `MapObject_GetScriptID(mapObject)`
  when the player presses A while facing the trainer.
- `scr_seq_0953_normalTrainer` already handles introductory and defeated
  dialogue, trainer-defined double battles, battle results, phone interactions,
  trainer flags, and rematches.

No DSPRE script command, variable, flag, message, trainer ID, map ID, event ID,
or object ID is added by the line-of-sight patch itself. The separate generated
team/reward offer is documented in `offer-a-shared-trainer-battle.md`.

## Minimal pattern

Patch the beginning of `TryGetSeenByNpcTrainers` in place to return `FALSE`:

```asm
.org 0x020640C8
.area 4
mov r0, #0
bx lr
.endarea
```

Keep direct interaction routed through the shared trainer script. Trainer
records that define double battles remain double. Separate trainers that were
combined only by automatic sight detection are intentionally fought one at a
time.

## Control-flow checklist

- Detection returns before pausing the player or starting a script.
- Direct interaction still launches the object's original script.
- No script lock, message, fade, task, or trainer flag is created by the patch.
- The patch asserts the expected original or already-patched Thumb instructions
  before writing, so an incompatible ROM layout fails the build.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. Verify an
undefeated ordinary trainer does not react at any distance, shows its offer
when spoken to, retains defeated dialogue afterward, and follows its rematch
flow. Also verify a trainer-defined double battle remains double and two
trainers with overlapping former sight lines can be fought separately.
