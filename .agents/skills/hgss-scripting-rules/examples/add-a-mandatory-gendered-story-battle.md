# Add a mandatory gender-selected HGSS story battle

## Goal

Insert a mandatory battle into an existing story scene, choose the opponent
from the player's gender, and prevent the scene reward and completion state
from advancing after a loss.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_revised_opening.s`
- Script member: 231, Route 31–Violet gatehouse
- Trigger: the existing coordinate event controlled by `VAR_UNK_4132`

## Verified HGSS commands

- `gender_msgbox 2, 3` preserves the original Lyra/Ethan message selection
  from member 231.
- `get_player_gender VAR_SPECIAL_RESULT` uses the same parameter ordering as
  common script 29 in `armips/scr_seq/scr_seq_00003_commonscript.s`.
- `compare` and `goto_if_eq` follow the same gender branch used by that common
  script. Gender 0 is the male player and therefore selects Lyra; gender 1 is
  the female player and selects Ethan.
- `trainer_battle TRAINER_ID, 0, 0, 0`, `check_battle_won`, and `white_out`
  copy the mandatory Silver battle pattern documented in
  `reward-a-scripted-trainer-victory.md`. Keeping the third battle parameter
  at zero prevents a loss from being treated as an allowed story outcome.
- `GiveItemNoCheck ITEM_VS_RECORDER, 1` is replayed only on the victory path,
  preserving the original gift command and parameter ordering.

## Verified identifiers

- Trainer 738 is Lyra and trainer 739 is Ethan in `data/Trainers.c`.
- Member 231 is the Route 31–Violet gatehouse script selected by map header
  `MAP_ROUTE_31_VIOLET_GATEHOUSE`.
- The original entry uses messages 2/3 for the counterpart introduction,
  messages 4/5 for the Vs. Recorder explanation, and messages 6/7 for the
  companion departure.
- `VAR_UNK_4132` is the existing scene variable. The original script sets it
  to 1 only at the end of the completed departure, so the loss branch must not
  write it.
- The gatehouse companion is object 2 in event bank 94. Like the other
  counterpart companion objects, it uses static sprite tag 1032.

## Minimal pattern

```asm
gender_msgbox LYRA_MESSAGE, ETHAN_MESSAGE
closemsg
get_player_gender VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_eq battle_lyra
trainer_battle TRAINER_ETHAN, 0, 0, 0
goto battle_finished
battle_lyra:
trainer_battle TRAINER_LYRA, 0, 0, 0
battle_finished:
check_battle_won VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_ne battle_won
white_out
releaseall
end
battle_won:
// Continue with the original reward and completion flow.
```

## Control-flow checklist

- The field remains locked while the battle is active.
- A loss calls normal blackout handling, releases the script lock, and ends
  without giving the item or changing the scene variable.
- A win returns to the original reward and departure flow.
- The battle IDs have distinct trainer classes so the correct front sprite,
  name, and gender appear.
- The replacement branch guards the expected original opcode and messages.
  Appended code must not overlap the original member or another patch.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. Check both
player genders. Verify the matching opponent appears, losing blacks out and
requires a rematch, winning gives exactly one Vs. Recorder, and re-entering
the gatehouse does not repeat the completed scene.
