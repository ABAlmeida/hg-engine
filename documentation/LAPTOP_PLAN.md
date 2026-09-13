# Laptop PC Access Plan

## Intended behavior

Add a reusable Laptop Key Item that provides portable access to the same
top-level PC menu used by physical Pokémon Center terminals.

The Laptop:

- opens Bill's or Someone's PC and the player's PC through the shared menu;
- works in ordinary indoor and outdoor field locations;
- returns to the overworld after the player logs off;
- is never consumed, tossed, registered, or usable in battle; and
- is initially given by Professor Elm immediately after the Healing Kit.

The Laptop is independent from permanent-death processing. It reuses the
shared PC menu, so later PC entries automatically remain consistent between
physical terminals and portable access.

Implementation status: implemented in source; build and focused in-game
verification remain pending. The temporary icon reuses the Rotom Catalog
art, item ID 2708 is assigned to `ITEM_LAPTOP`, and common script 2076 enters
the same PC menu body while suppressing only physical-terminal animations.

## 1. Add the Key Item

Choose a unique `ITEM_LAPTOP` ID and add its item data, name, description,
article forms, Bag pocket assignment, price, and a temporary source-controlled
icon.

Give it a dedicated field-use callback. It must use the normal Key Item Bag
exit flow, must not be consumable or throwable, and must not be registrable or
usable from the battle Bag.

## 2. Centralize PC menu startup

Identify the field operation used by physical PC terminals to launch the
top-level PC menu. Extract or wrap the shared startup behavior behind one
narrow entry point that accepts the return context required by the caller.

Both physical terminals and the Laptop must use that operation. Do not copy
the PC menu or maintain a second list of entries in the Laptop implementation.

The Laptop owns only portable entry. Bill's PC, the player's PC, and any later
PC option continue to own their respective behavior.

## 3. Validate safe field use

Allow use on normal routes, in caves, inside buildings, and in other ordinary
field locations.

Reject contexts that cannot safely suspend into the PC overlays, including:

- an active link or wireless session;
- another active field application;
- a script or transition state that cannot yield to an overlay; and
- any other context found by tracing the physical PC launcher's lifetime
  requirements.

Use the standard field-item rejection response. Do not start the overlay and
then attempt to recover from an invalid context.

## 4. Preserve Bag and field lifetimes

The field-use task must:

1. validate the context;
2. close the Bag through its normal Key Item flow;
3. launch the shared PC menu only after Bag teardown is complete;
4. retain only pointers whose owners outlive the PC overlay;
5. wait for the complete PC menu and child-overlay session to finish; and
6. restore normal field control.

Avoid a global pending-Laptop flag. Keep the state in the existing field task
or item-use environment for its complete lifetime.

## 5. Add Elm's initial gift

Professor Elm gives the Laptop during the existing one-time post-starter gift
sequence in his lab, immediately after he gives the Healing Kit.

His script is member 843. Extend the source-controlled Healing Kit hook with a
second standard item receipt before the revised opening's Egg and phone-number
sequence. The starter-selection progression already makes this gift path
one-time, so no separate Laptop acquisition flag is needed.

The Laptop remains a normal Key Item and must be added to the Bag.

Update message bank 543 so Elm introduces the Laptop naturally. Keep the gift
inside the existing guarded, source-controlled Healing Kit patch with its
assertions against the supported US HeartGold base.

## 6. Performance and maintainability

Laptop use is an infrequent field action, so its direct performance cost is
negligible. The important risks are task and overlay lifetime correctness.

Keep these responsibilities separate:

- the item callback validates use and exits the Bag;
- the shared launcher owns PC-menu startup and completion;
- each PC menu entry owns its own functionality; and
- Elm's post-starter gift script owns acquisition only.

Do not duplicate PC scripts, overlay setup, permanent-death rules, or storage
access inside the Laptop callback.

## 7. Resolved implementation mapping

The implementation uses:

1. custom item ID 2708 and the Rotom Catalog art as the temporary icon;
2. common script 2010's existing `_0A2E` PC menu body, reached from portable
   common script 2076;
3. the same deferred Bag-exit field task pattern as other custom Key Items;
4. the field system's ordinary-overworld map-load type as the safe-use gate;
5. a per-script scratch mode that suppresses terminal model commands while
   preserving every shared child-overlay return path; and
6. messages 112 and 113 in bank 543, inserted immediately after the Healing
   Kit receipt in Elm's member-843 post-starter gift script.

## 8. Logical implementation steps

1. Add the Laptop item data, text, icon, and field-use registration.
2. Centralize the physical-PC menu launcher.
3. Implement safe deferred Laptop launch and return-to-field behavior.
4. Add Elm's guarded gift and dialogue changes.
5. Review the complete diff and update project status documentation.

## 9. Manual verification

Verify:

1. Elm gives exactly one Laptop immediately after the Healing Kit;
2. the Laptop remains in the Key Items pocket and is not consumed;
3. it cannot be tossed, registered, or used in battle;
4. invalid field contexts produce the standard rejection without hanging;
5. it works on ordinary routes, caves, and indoor maps;
6. the Bag closes cleanly before the PC menu appears;
7. logging off returns to the correct field state with player control;
8. physical terminals and the Laptop expose the same top-level menu;
9. every child PC entry returns correctly through the Laptop path;
10. repeated Laptop use does not leak tasks, overlays, or stale state; and
11. save and reload retain the Laptop and all PC changes normally.

Codex does not run the Heartless Gold build, tests, or emulator. Build and
focused in-game verification remain user-run activities.
