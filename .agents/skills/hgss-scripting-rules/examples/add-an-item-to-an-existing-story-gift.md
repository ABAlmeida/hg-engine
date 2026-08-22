# Add an item to an existing HGSS story gift

## Goal

Give 5 Potions, 100 Poké Balls, 100 Poké Bait, and the Fishing Rod through
Elm's assistant's opening supply sequence, while preserving an all-or-nothing
Bag-capacity check and the vanilla Rod ownership state.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_00843_elm_healing_kit.s`
- Script member: 843, replacement for table entry 3
- Trigger: the assistant who stops the player at Elm's lab exit

## Verified HGSS commands

- `goto_if_no_item_space ITEM_OLD_ROD, 1, target` uses the established
  `hasspaceforitem` wrapper also used for the 5-Potion, 100-Poké-Ball, and
  100-Poké-Bait stacks in this same sequence. Its parameters are item,
  quantity, and failure label.
- `setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD` followed by
  `setvar VAR_SPECIAL_x8005, 1` and `callstd std_obtain_item_verbose` uses the
  verified item/quantity input order shared by repository item gifts.
- `setflag FLAG_GOT_OLD_ROD` is the vanilla ownership flag. Its original gift
  is member 233 (`scr_seq_0233_R32PC0101.s`). Set it only after the verbose
  item award succeeds.

## Verified identifiers

- Item: `ITEM_OLD_ROD` (displayed as Fishing Rod) from the generated Armips
  item constants.
- Flag: `FLAG_GOT_OLD_ROD` (117) from `armips/include/flags.s`.
- Object: Elm's assistant is object 2 in member 843, already used by the
  existing replacement sequence.
- Messages: member 843's message bank is `data/text/543.txt`; local message 21
  describes use of the supplies and Fishing Rod.
- Trainer IDs, map IDs, and new variables: Not used.

## Minimal pattern

```asm
goto_if_no_item_space ITEM_FIRST_GIFT, FIRST_QUANTITY, bag_full
goto_if_no_item_space ITEM_OLD_ROD, 1, bag_full

// Award every earlier gift here.
setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setflag FLAG_GOT_OLD_ROD

// Advance the owning scene only after every gift succeeds.
```

## Control-flow checklist

- Check all gifts before awarding the first one. A failed retry must not
  duplicate supplies already received during a partial attempt.
- Set `FLAG_GOT_OLD_ROD` only after the Rod enters the Bag.
- Advance the lab scene only after all gifts succeed.
- Preserve the existing assistant movements, `lockall`/`releaseall`, Bag-full
  response, and termination paths.
- Do not add a second Fishing Rod gift elsewhere; restore the displaced story
  sequence at that location.

## Replace an existing story reward in place

Elm's post-hatch reward in member 843 demonstrates the smaller case where the
existing trigger and item-gift flow should remain intact. At offset `0x5F2`,
the verified `SetVar VAR_SPECIAL_x8004, ITEM_EVERSTONE` begins the standard
Bag-space and verbose-gift sequence. Heartless Gold asserts the opcode,
variable, old-or-already-patched item, and following quantity assignment, then
changes only the item operand at `0x5F6` to `ITEM_EVIOLITE`. This preserves the
Togepi/Togetic/Togekiss checks, Bag-full branch, messages, and completion flags
without duplicating the routine.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. On a new
save, verify the assistant gives all four supplies exactly once, the Rod opens
the fishing flow, a full relevant pocket produces no partial award, retrying
after making room succeeds, and the Route 32 fisherman recognizes the existing
ownership flag. A successful build does not prove script flow.
