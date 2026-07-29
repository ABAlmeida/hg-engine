# Reusable Healer Plan

## Intended behavior

Add a starting, reusable Key Item that:

- fully restores the HP of every valid, non-Egg party Pokémon, including
  reviving fainted Pokémon;
- clears persistent major status conditions;
- restores every move to its calculated maximum PP, including PP Up and
  PP Max bonuses;
- is never consumed, cannot be tossed, and cannot be used in battle; and
- always reports that the party was healed, even when no party member needed
  healing.

Using the item follows the standard Key Item flow: the Bag closes, the healing
operation runs in the field, displays `Your Pokémon were fully healed!`, and
returns control to the overworld.

Eggs are skipped. Healing must not change moves, PP Up values, friendship,
effort values, held items, Pokérus, or other unrelated Pokémon data.

The visible item name is `Healing Kit`, with the Full Restore icon used
temporarily.

## 1. Add the item

Add a unique item ID after Shiny Bait and update `MAX_TOTAL_ITEM_NUM`.

Define item data with:

- a price of zero;
- `POCKET_KEY_ITEMS` as its field pocket;
- tossing disabled;
- no held-item or battle effect;
- no battle pocket; and
- a dedicated field-use function ID.

Add the name, description, article forms, and temporary icon entry to the
source-controlled item text and graphics data.

## 2. Centralize party healing

Add a small reusable-healer module with one authoritative operation:

```c
void ReusableHealer_HealParty(struct Party *party);
```

For each valid, non-Egg party member, it will:

1. set current HP to maximum HP;
2. clear the persistent status field; and
3. call the existing `RestoreBoxMonPP()` routine so restored PP correctly
   respects PP Ups.

The operation does not need to determine whether healing was necessary.
Calling it on an already healthy party is valid and follows the same success
path.

## 3. Use the standard Key Item flow

Add a dedicated entry to `sItemFieldUseFuncs` and keep its enum value and the
table's static size assertion synchronized.

`ItemMenuUseFunc_ReusableHealer` will:

1. close the Bag through the existing field-task exit state;
2. obtain the saved party from the field-system context;
3. call `ReusableHealer_HealParty()`;
4. display `Your Pokémon were fully healed!` through the game's standard
   field-side Key Item message task; and
5. return control to the overworld.

This path must not set global transient state or call `Bag_TakeItem()`. The
small message allocation is owned and released by the standard message task.

There is no no-effect check. An empty or already healthy party still receives
the normal success message.

The field message reuses the base game's internal
`Task_PrintRegisteredKeyItemUseMessage` routine at the US HeartGold address
recorded in `rom.ld`. Its private task-data layout is documented locally and
guarded by a size assertion. This fixed-address dependency must be rechecked
if the supported base ROM changes.

## 4. Leave registered-item use disabled

Set the item as non-selectable so it cannot be registered. Registered use would
require a second entry path solely for the shortcut and is not needed for the
initial feature.

## 5. Receive it from Professor Elm

Professor Elm gives one copy during the one-time starter sequence, after he
explains that the lab's machine can heal the player's Pokémon. The gift uses
the standard item receipt flow before Elm resumes the rest of his instructions.
Do not add a save field.

The grant must:

- occur after the player has received and optionally nicknamed their starter;
- not be repeated when that save is loaded; and
- not be backfilled into saves that have already completed the starter
  sequence.

The existing `FLAG_GOT_STARTER` gate makes the sequence one-time, so acquisition
does not need a second flag. A guarded source-controlled patch extends Elm's
script member and asserts the expected original commands before redirecting
execution. The patch replays the displaced movement command and returns to the
original continuation. Elm's updated dialogue is source-controlled as message
bank 543 in `data/text/543.txt`.

The script-member offsets are a second base-ROM dependency. They are named,
documented, and protected by byte assertions so an incompatible script fails
the build instead of being patched silently.

Guard acquisition and use behind `IMPLEMENT_REUSABLE_HEALER`. When disabled,
Elm does not give the item and its callback performs no action.

A save made before choosing a starter follows the normal starter sequence and
therefore receives the item. Existing saves already past that sequence are not
migrated.

## 6. Verification

Create an incremental ROM:

```sh
make quick-rom -j$(nproc)
```

Codex must not run the Heartless Gold build, test suite, or emulator. The user
runs the build and focused manual verification.

Manually verify with a new in-game save that:

1. the item is absent before choosing a starter;
2. all three starter choices reach Elm's Healing Kit dialogue and gift;
3. accepting and declining the nickname prompt both reach the gift;
4. the item is added once to the Key Items pocket;
5. Elm's subsequent movement and dialogue complete normally;
6. talking to Elm again does not award another copy;
7. using it closes the Bag and returns to the overworld after the message;
8. the success message also appears for an already healthy party;
9. damaged and fainted Pokémon return to maximum HP;
10. every persistent major status is cleared;
11. all move PP is restored and PP Up bonuses are respected;
12. multiple party members are healed in one use;
13. Eggs and unrelated Pokémon data remain unchanged;
14. the item is never consumed and cannot be tossed or used in battle;
15. the item cannot be registered;
16. the item remains present after saving and reloading; and
17. Pokémon Center healing continues to work normally.

Implementation and focused in-game verification are complete.
