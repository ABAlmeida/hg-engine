# Offer a shared trainer battle

## Goal

Show an undefeated shared-script trainer's generated team and configured
reward, then allow the player to accept or decline the battle.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_00953_trainerscript.s`
- Script label: `scr_seq_0953_normalTrainer`
- Trigger: talking to an undefeated trainer whose event uses the shared normal
  trainer script

## Verified HGSS commands

- `get_trainer_num VAR_SPECIAL_x8004` obtains the interacting trainer ID in the
  existing shared script.
- `trainer_message TRAINER_VAR, MESSAGE_TYPE` displays a trainer-text entry.
  Existing calls in this script use message types returned by
  `get_trainer_msg_params`; the generated offer uses `TRMSG_BATTLE_OFFER`.
- `yesno VAR_SPECIAL_RESULT` follows the verified common-script pattern in
  `scr_seq_0003_020`: zero accepts and one declines.
- `hasspaceforitem ITEM_VAR, QUANTITY_VAR, RESULT_VAR` and
  `callstd std_bag_is_full` copy the existing item-gift checks in this script.
- `callstd std_obtain_item_verbose` uses the existing standard item award flow.
- `trainer_battle`, `check_battle_won`, `settrainerflag`, `releaseall`, and
  `end` retain their existing parameter order and control flow in the shared
  trainer script.

## Verified identifiers

- `VAR_SPECIAL_x8004` holds the trainer ID before reward lookup and the item ID
  while running standard item commands.
- `VAR_SPECIAL_x8005` holds the generated reward quantity; zero means no reward.
- `VAR_TEMP_x4012` already preserves the trainer ID across the post-battle flow.
- `TRMSG_BATTLE_OFFER` is trainer message type 11 in `include/trainer_data.h`.
- Trainer and item IDs come from `data/trainer_rewards.csv`; the build validates
  them against generated trainer data and `include/constants/item.h`.
- Map, event, NPC, and object IDs: not used; this changes the shared trainer
  entry rather than individual map events.

## Minimal pattern

```asm
trainer_message VAR_SPECIAL_x8004, TRMSG_BATTLE_OFFER
yesno VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 1
goto_if_eq decline_battle

// Continue through the existing reward-space check and trainer battle.

decline_battle:
closemsg
releaseall
end
```

## Control-flow checklist

- The defeated check runs before the offer, so defeated dialogue and rematches
  retain their original paths.
- Declining balances the existing `lockall` with `releaseall` and changes no
  trainer or reward state.
- A configured reward is checked for Bag space before battle and awarded only
  after a win.
- Loss still branches directly to the existing `white_out` path.
- Automatic and custom-script battles do not enter this shared offer path.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. Verify a
first-time route trainer can be declined repeatedly, accepting starts the
correct single or trainer-defined double battle, a win grants one configured
reward, a loss grants none, a full Bag prevents the battle, defeated dialogue
still works, and rematches retain their original interaction.
