# Laptop PC Access Plan

## Intended behavior

Add a reusable Laptop Key Item that provides portable access to the same
top-level PC menu used by physical Pokémon Center terminals.

The Laptop:

- opens Bill's or Someone's PC and the player's PC through the shared menu;
- works in ordinary indoor and outdoor field locations;
- returns to the overworld after the player logs off;
- is never consumed, tossed, registered, or usable in battle; and
- is initially given by Mum during the existing Pokégear sequence.

The Laptop is independent from permanent-death processing. It reuses the
shared PC menu, so later PC entries automatically remain consistent between
physical terminals and portable access.

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

## 5. Add Mum's initial gift

For the initial implementation, Mum gives the Laptop during the existing
Pokégear sequence in the player's house.

Her script is member 845 (`T20R0201`). Insert the standard item receipt after
the Pokégear is granted and before the remaining Pokégear explanation.
`FLAG_GOT_POKEGEAR` already makes the sequence one-time, so no separate Laptop
acquisition flag is needed.

The Pokégear is enabled by a story flag, whereas the Laptop is a normal Key
Item and must be added to the Bag.

Update message bank 545 so Mum introduces the Laptop naturally. Implement the
gift and dialogue as guarded, source-controlled script patches with assertions
against the supported US HeartGold base. This acquisition point is
intentionally provisional and may be moved later.

## 6. Performance and maintainability

Laptop use is an infrequent field action, so its direct performance cost is
negligible. The important risks are task and overlay lifetime correctness.

Keep these responsibilities separate:

- the item callback validates use and exits the Bag;
- the shared launcher owns PC-menu startup and completion;
- each PC menu entry owns its own functionality; and
- Mum's script owns acquisition only.

Do not duplicate PC scripts, overlay setup, permanent-death rules, or storage
access inside the Laptop callback.

## 7. Remaining pre-implementation mapping

Before changing runtime code:

1. select an unused item ID and the temporary icon source;
2. trace the physical PC script command and field task into the top-level PC
   menu;
3. identify the Bag field-use teardown and deferred-application pattern used
   by comparable Key Items;
4. document the unsafe-context checks required by the shared PC launcher;
5. map the return path from every PC child overlay back to the field;
6. verify the exact Pokégear gift insertion point in script member 845; and
7. identify the corresponding lines in message bank 545.

## 8. Logical implementation steps

1. Add the Laptop item data, text, icon, and field-use registration.
2. Centralize the physical-PC menu launcher.
3. Implement safe deferred Laptop launch and return-to-field behavior.
4. Add Mum's guarded gift and dialogue changes.
5. Review the complete diff and update project status documentation.

## 9. Manual verification

Verify:

1. Mum gives exactly one Laptop during the Pokégear sequence;
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
