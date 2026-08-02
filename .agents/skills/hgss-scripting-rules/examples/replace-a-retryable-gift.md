# Replace a one-time gift while preserving retry behavior

## Goal

Replace Violet's post-Falkner Togepi Egg with one Shiny Bait without adding a
new save flag.

## Target and verified commands

- Source: `armips/scr_seq/scr_seq_revised_opening.s`
- Member 858, entry 2
- `goto_if_no_item_space ITEM_SHINY_BAIT, 1, target` and
  `callstd std_obtain_item_verbose` copy the established HGSS item-gift pattern
  defined in `armips/include/scriptmacros.s`.
- The assistant object ID, departure movements, hide flags, Violet scene 3,
  and Elm lab scene 7 come from vanilla member 858.

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

