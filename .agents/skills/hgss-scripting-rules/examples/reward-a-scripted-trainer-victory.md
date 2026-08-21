# Reward an HGSS scripted trainer victory

## Goal

Award an item after a won scripted trainer battle without awarding it after a
loss or adding the item to the opposing trainer's active battle-item array.

## Target

- Silver 1 source: `armips/scr_seq/scr_seq_revised_opening.s`, member 842,
  `silver_one_after_lab`.
- Falkner source: `armips/scr_seq/scr_seq_00859_falkner_iv_max.s`, member 859,
  `T22GYM0101`.
- Bugsy source: `armips/scr_seq/scr_seq_00869_bugsy_iv_max.s`, member 869,
  `T23GYM0102`.
- Proton source: `armips/scr_seq/scr_seq_00060_proton_reward.s`, member 60,
  `D26R0102`.
- Trigger: a victory in the applicable scripted trainer battle.

## Verified HGSS commands

- `check_battle_won VAR_SPECIAL_RESULT`, followed by `compare`, copies the
  outcome check used by the shared trainer script. Zero is the loss result;
  this script uses `goto_if_ne` to branch to the victory path.
- `trainer_battle TRAINER_ID, 0, 0, 0` copies the ordinary single-trainer
  battle parameters from the shared trainer script. Its third parameter must
  remain zero here: setting it to one marks the battle as `can lose`, causing
  the encounter task to heal the party and restore the overworld before the
  field script resumes. That recovery path cannot then be followed by the
  normal `white_out` transition.
- `white_out` is command 219 in `armips/include/scriptmacros.s`. The shared
  normal-trainer and sight-trainer scripts call it after a zero battle result,
  followed by `releaseall` and `end`.
- `setvar VAR_SPECIAL_x8004, ITEM_ORAN_BERRY` supplies the item ID and
  `setvar VAR_SPECIAL_x8005, 1` supplies the quantity consumed by
  `callstd std_obtain_item_verbose`. Repeating these three commands with the
  next item is verified by the consecutive Potion, Poké Ball, and Poké Bait
  awards in `armips/scr_seq/scr_seq_00843_elm_healing_kit.s`.
- `callstd std_obtain_item_verbose` is standard script 2008 in
  `armips/include/scriptmacros.s`. It adds the item and presents the normal
  obtained-item message.
- `goto_if_no_item_space ITEM, QUANTITY, TARGET` sets `VAR_SPECIAL_x8004` and
  `VAR_SPECIAL_x8005`, checks Bag capacity, and branches when the item cannot
  fit. Its parameter order and use before `std_give_item_verbose` are copied
  from Falkner's original TM51 reward in member 859.
- `giveitem_no_check ITEM, QUANTITY` resets those item variables and calls
  `std_give_item_verbose`. Falkner uses it only after both TM51 and IV Max have
  passed their capacity checks.

## Verified identifiers

- Trainer IDs 495, 496, and 497 are the three Silver 1 variants selected by
  `silver_one_after_lab` and documented in
  `documentation/guides/CHANGING_TRAINER_POKEMON.md`.
- `ITEM_ORAN_BERRY` is item 155 in both `asm/include/items.inc` and
  `include/constants/item.h`.
- `ITEM_IV_MAX` is the custom item at `MAX_BASE_ITEM_NUM + 22` in both item
  constant files.
- Message 39, Silver's object ID, its departure movement, the scene variables,
  and completion flags were already used by this same script entry. No new
  variable, flag, message, map, event, or object ID is introduced.
- Member 859 is Violet Gym (`T22GYM0101`). Trainer 20 is Falkner, and
  `FLAG_GOT_TM51_FROM_FALKNER` is the original retry/completion state for his
  post-victory TM51 handoff. The IV Max extension deliberately reuses that
  state instead of allocating another flag.

## Minimal pattern

```asm
check_battle_won VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_ne victory
white_out
releaseall
end

victory:
npc_msg 39
setvar VAR_SPECIAL_x8004, ITEM_ORAN_BERRY
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_IV_MAX
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
closemsg
releaseall
end
```

Falkner's two-pocket, retryable reward uses this variation:

```asm
goto_if_no_item_space ITEM_TM051, 1, bag_full
goto_if_no_item_space ITEM_IV_MAX, 1, bag_full
giveitem_no_check ITEM_TM051, 1
giveitem_no_check ITEM_IV_MAX, 1
setflag FLAG_GOT_TM51_FROM_FALKNER
// Continue the original post-gift dialogue and cleanup.
```

## Control-flow checklist

- The loss branch bypasses the reward and invokes normal blackout handling
  before releasing the field lock.
- The item is awarded before the script advances Silver's completed scene
  state, so ordinary execution cannot revisit the reward.
- Silver 1 occurs before normal play can fill the Berry or Medicine pocket, so
  a retry branch is not added to this mandatory cutscene. A modified save with
  either pocket full remains outside this opening assumption.
- When the central trainer-reward system is implemented, move IDs 495-497 into
  its external table and remove this script award in the same change.
- Falkner must remain in his custom Gym script unless that script explicitly
  calls a reusable reward routine. Adding trainer 20 to `trainer_rewards.csv`
  alone cannot award the item because his battle bypasses the shared ordinary
  trainer flow.
- Falkner checks both pockets before giving either item. TM51 and IV Max use
  different pockets, so the two independent capacity checks make the bundle
  atomic without additional save state. The existing TM51 flag remains unset
  on the Bag-full path, allowing the player to talk to Falkner and retry.
- Bugsy uses the same atomic two-pocket pattern with TM89 and IV Max, reusing
  `FLAG_GOT_TM89_FROM_BUGSY` so a Bag-full result remains retryable.
- Proton has no later retry conversation. His won-battle branch awards the
  Amulet Coin before resuming the original post-battle dialogue and story
  cleanup; the loss branch remains untouched.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. For Silver
1, verify each starter matchup uses the correct level-5 opposing starter, none
of the three variants knows a same-type attack or holds an item, victory grants
exactly one Oran Berry and one IV Max, loss grants neither, and the battle
cannot be repeated for another reward after save/reload. For Falkner, verify a
victory grants TM51 and one IV Max, later conversations grant neither again,
and filling either relevant pocket produces the Bag-full response and permits
a successful retry after space is made.
