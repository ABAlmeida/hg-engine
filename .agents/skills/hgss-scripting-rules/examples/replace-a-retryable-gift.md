# Replace a one-time gift while preserving retry behavior

## Goal

Replace a one-time gift with Shiny Bait while preserving full-Bag retry and
preventing a consumable replacement from becoming repeatable.

## Target and verified commands

- Source: `armips/scr_seq/scr_seq_revised_opening.s`
- Member 858, entry 2
- `goto_if_no_item_space ITEM_SHINY_BAIT, 1, target` and
  `callstd std_obtain_item_verbose` copy the established HGSS item-gift pattern
  defined in `armips/include/scriptmacros.s`.
- The assistant object ID, departure movements, hide flags, Violet scene 3,
  and Elm lab scene 7 come from vanilla member 858.
- Source: `armips/scr_seq/scr_seq_removed_rod_rewards.s`
- Member 918's Good Rod item operand is replaced in place because its existing
  `FLAG_GOT_GOOD_ROD` check, capacity check, and post-award flag update already
  provide the required one-time flow.
- Member 200's Super Rod flow originally used possession of the permanent Key
  Item as completion state. Its replacement uses `VAR_GOT_ROUTE_12_SHINY_BAIT`
  (`0x416F`), verified absent from every current script-archive member and all
  source references before being named for this purpose.
- `yesno VAR_SPECIAL_RESULT` and the convention that result 1 is the declined
  branch are copied from the existing HGSS common scripts.

## Minimal pattern

```asm
goto_if_no_item_space ITEM_SHINY_BAIT, 1, bag_full
setvar VAR_SPECIAL_x8004, ITEM_SHINY_BAIT
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
// Only now advance the original one-time scene state.
end

bag_full:
callstd std_bag_is_full
closemsg
releaseall
end
```

## Manual verification

Verify a full-Bag refusal leaves the assistant available, a later retry awards
exactly one item, and map reload/save-load cannot repeat a successful gift.
For a gift that originally relied on possession of an unconsumable Key Item,
do not merely replace both item checks with the consumable reward: doing so
would allow another reward after use. Assign verified-free persistent state,
set it only after the award succeeds, and document how its availability was
established.
