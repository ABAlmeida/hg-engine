# Graveyard and Laptop Plan

## Intended behavior

Add a permanent-retirement system for fainted Pokémon and portable access to
the game's PC:

- the Graveyard contains 10 boxes, for a maximum of 300 Pokémon;
- it appears as a third top-level PC option, separate from the player's PC and
  Bill's or Someone's PC;
- Pokémon in the Graveyard can be viewed but cannot be withdrawn, deposited
  into the party, transferred to ordinary PC boxes, released, or have held
  items manipulated;
- after a battle has completely written its results back to the saved party,
  every fainted party Pokémon is immediately removed from the party and placed
  in the first empty Graveyard slot;
- if all 300 Graveyard slots are occupied, newly retired Pokémon are
  permanently discarded;
- after a full-party wipe, the first usable Pokémon in ordinary PC storage is
  moved into the party; and
- a Laptop Key Item opens the same PC menu without requiring a Pokémon Center.

The rules are active immediately. Existing saves do not need migration support;
implementation and manual verification should use a new save.

## 1. Keep Graveyard storage separate

Do not increase `NUM_PC_BOXES` from 30 to 40. The existing incremental PC save
writer uses one `u32` modified-box mask, so it cannot represent modifications
to more than 32 ordinary boxes.

Add a dedicated `GraveyardStorage` save region containing:

- 10 `PC_BOX` records;
- an active-view box index;
- a 10-bit modified-box mask stored in a suitably sized integer;
- fixed Graveyard box names; and
- any minimal wallpaper state required by the reused PC viewer.

Keep `NUM_PC_BOXES` at 30 and define independent constants such as:

```c
#define NUM_GRAVEYARD_BOXES 10
#define GRAVEYARD_CAPACITY (NUM_GRAVEYARD_BOXES * MONS_PER_BOX)
```

Provide narrow operations instead of exposing the layout broadly:

```c
BOOL Graveyard_StoreMon(GraveyardStorage *graveyard, const struct BoxPokemon *mon);
struct BoxPokemon *Graveyard_GetMon(GraveyardStorage *graveyard, u32 box, u32 slot);
```

The storage operation searches boxes 0 through 9 and slots 0 through 29. It
returns `FALSE` when full; the caller then permanently discards the Pokémon as
required.

## 2. Extend the expanded save deliberately

Allocate 10 additional 0x1000-byte box pages in the expanded save rather than
placing roughly 40 KiB inside an unrelated small save structure.

The save changes must:

1. increase the dynamic-region, full-save, and heap-offset constants together;
2. update every patched `SaveData` field offset affected by the larger region;
3. initialize all Graveyard boxes on new-save creation;
4. load and validate the Graveyard region with its own CRC state;
5. write only modified Graveyard boxes during an ordinary incremental save;
6. include the Graveyard footer and dirty flags in rollback and failure paths;
7. initialize both redundant save slots consistently; and
8. add compile-time size and offset assertions for the new layouts.

The regular PC and Graveyard need separate dirty masks. Do not widen or
overload `boxModifiedFlag`, because existing code and fixed-address consumers
expect the ordinary PC mask to remain a `u32`.

This is the highest-risk part of the feature. All fixed save offsets in
`include/constants/save.h`, `bytereplacement`, `SaveData`, and the asynchronous
writer must be audited as one change.

## 3. Centralize post-battle retirement

Add one authoritative operation:

```c
GraveyardResult Graveyard_ProcessPartyAfterBattle(SaveData *saveData);
```

Run it only after battle results have been synchronized into the saved party,
but before ordinary blackout recovery can heal or otherwise replace fainted
party members.

The operation will:

1. inspect the saved party, not a temporary battle-party copy;
2. copy every Pokémon with zero current HP into a temporary `BoxPokemon`;
3. try to place that copy in the first empty Graveyard slot;
4. permanently discard the copy if the Graveyard is full;
5. remove the original from the party through the existing party-removal
   operation; and
6. compact the party without skipping adjacent fainted members.

Process party slots from the end toward the beginning so removal cannot change
the index of an unprocessed Pokémon.

The Graveyard stores the normal boxed representation. It does not add a second
"dead" flag to Pokémon data: presence in Graveyard storage is the authoritative
retirement state.

## 4. Use a general encounter-completion hook

The existing encounter-finish hook currently also applies level-cap rewards.
Refactor it into a general post-battle dispatcher instead of making Graveyard
logic depend on the level-cap module.

The dispatcher should:

1. preserve the base game's result-variable and return behavior;
2. apply trainer-victory level-cap rewards when appropriate;
3. process fainted saved-party Pokémon for every completed battle route that
   writes damage back to the party;
4. recover from a wipe if retirement leaves the party empty; and
5. return to the original field or blackout flow.

Audit ordinary wild and trainer battles, scripted encounters, captures,
fleeing, partner and double battles, the Bug-Catching Contest, Safari
encounters, facilities, and link battles. Routes using temporary parties or
restoring the party before returning should be documented explicitly rather
than forced through unsafe assumptions.

Do not run this from a move-end, faint animation, or battle-controller hot
path. Those locations can execute more than once and may still contain battle
copies that have not been committed to the save.

## 5. Recover after a wipe

After all fainted party members have been retired, check whether the party is
empty.

If it is empty:

1. scan ordinary Bill's PC in deterministic box-major order: box 0, slot 0
   through box 29, slot 29;
2. select the first valid, non-Egg Pokémon;
3. convert it through the same boxed-to-party path used by an ordinary
   withdrawal;
4. add it to the party;
5. remove it from ordinary PC storage; and
6. mark the affected ordinary box as modified.

Eggs are skipped because they cannot recover the player into a usable battle
state. If ordinary storage contains only Eggs, treat it as having no available
Pokémon.

The transfer must be transactional: do not delete the boxed Pokémon unless the
party addition succeeds.

## 6. Use a dedicated defeat-to-title ending

If the player wipes without a usable Pokémon in ordinary PC storage:

1. finish Graveyard retirement and save-data mutation;
2. show a dedicated run-over message;
3. wait for acknowledgement;
4. fade out;
5. cleanly close the encounter and field-task state; and
6. return to the title screen without performing normal blackout recovery.

Do not invoke the normal credits sequence. It assumes Hall of Fame and story
state that does not exist during a post-battle wipe and would require a more
fragile standalone credits wrapper.

Do not reuse the normal blackout flow when no reserve exists; it expects a
party that can be healed and would undermine permanent retirement.

## 7. Add a restricted Graveyard PC view

Add `Graveyard` as a top-level option in the standard PC menu. It should be
available from the beginning of the game, whether the terminal currently says
Someone's PC or Bill's PC.

Reuse the existing Pokémon Storage System presentation where practical, but
launch it with an explicit Graveyard mode and Graveyard storage pointer.

Graveyard mode permits:

- moving between its 10 boxes;
- moving the cursor;
- viewing Pokémon summaries; and
- exiting to the PC menu.

It disables:

- party and ordinary-PC panes;
- picking up or moving Pokémon;
- withdrawal and deposit;
- release;
- held-item operations; and
- any shortcut that can mutate or export a Graveyard Pokémon.

Do not use an unscoped global mode bit. Pass the mode through the PC launch
environment and keep it valid for the complete overlay lifetime. Any required
overlay hooks must use named addresses, assertions, and comments explaining
the blocked mutation route.

## 8. Add the Laptop Key Item

Add a unique `ITEM_LAPTOP` ID, item data, text, article forms, a temporary icon,
and a dedicated field-use callback.

The Laptop:

1. closes the Bag through the standard Key Item exit flow;
2. launches the same top-level PC menu used by physical PC terminals;
3. includes the player's PC, Bill's or Someone's PC, and Graveyard;
4. returns to the overworld after logoff; and
5. is never consumed, tossed, registered, or usable in battle.

Centralize PC menu startup so the physical terminal and Laptop do not maintain
separate menu implementations.

Add a field-use check that rejects genuinely unsafe contexts such as an active
link session, another field application, or a scripted state that cannot
suspend into the PC overlays. Ordinary routes, caves, buildings, and outdoor
maps should remain valid.

For the initial implementation, Mum gives the Laptop during the existing
Pokégear sequence in the player's house. Her script is member 845
(`T20R0201`), and `FLAG_GOT_POKEGEAR` already makes that sequence one-time.
Insert the standard Laptop item receipt after the Pokégear is granted and
before the remaining Pokégear explanation.

The Pokégear itself is enabled by a story flag rather than added to the Bag;
the Laptop remains a normal Key Item and therefore needs its own item receipt.

Update Mum's message bank 545 where needed so the dialogue introduces the
Laptop naturally. Implement this as a guarded source-controlled script patch,
using the same assertion and reproducibility standards as Elm's Healing Kit
gift. No separate Laptop acquisition flag is needed.

This acquisition point is intentionally provisional and may be moved later.

## 9. Performance and maintainability

Post-battle work scans at most six party slots and, only for each retired
Pokémon, up to 300 Graveyard slots. This cost occurs once per completed battle
and is negligible.

Keep these concerns separate:

- save serialization owns Graveyard persistence;
- Graveyard storage owns placement and lookup;
- post-battle policy owns retirement and wipe recovery;
- PC presentation owns read-only viewing; and
- the Laptop owns only portable entry into the shared PC menu.

Avoid copying PC overlay logic into the Laptop or encoding retirement rules in
scripts. Keep all destructive decisions in one auditable C operation.

## 10. Implementation sequence

Implement and review the feature in independently verifiable stages:

1. Define the separate Graveyard layout and expanded-save constants.
2. Implement initialization, loading, incremental writing, CRC handling, and
   dirty tracking.
3. Add storage placement and read-only access operations.
4. Add centralized post-battle retirement without wipe recovery.
5. Add transactional ordinary-PC recovery after a wipe.
6. Implement the dedicated defeat-to-title ending.
7. Add the physical-PC Graveyard option and restricted viewer.
8. Add the Laptop item, Mum's Pokégear-sequence gift, and the shared portable
   PC launcher.
9. Add documentation and complete a full diff review.

Do not proceed to battle enforcement until Graveyard save/load behavior has
been manually verified. A storage failure here could permanently lose Pokémon
or corrupt the expanded save.

## 11. Manual verification

Use a new in-game save and verify:

1. an empty Graveyard survives save and reload;
2. each of its 10 boxes saves independently;
3. multiple adjacent fainted party Pokémon are all retired once;
4. retirement occurs after wins, losses, captures, and fleeing when fainted
   party members exist;
5. ordinary living party Pokémon are unchanged;
6. Graveyard order is deterministic;
7. a full Graveyard permanently discards later retirements without corrupting
   existing entries;
8. a wipe pulls the first non-Egg ordinary boxed Pokémon into the party;
9. a failed party addition leaves the boxed Pokémon intact;
10. a wipe with only boxed Eggs follows the no-reserve ending;
11. a wipe with no ordinary boxed Pokémon follows the no-reserve ending;
12. Graveyard entries can be viewed but cannot be mutated or exported through
    any PC command;
13. the physical PC and Laptop expose the same menu and storage contents;
14. the Laptop rejects unsafe field contexts without hanging;
15. the Laptop works on ordinary indoor and outdoor maps;
16. saving during later normal play retains both ordinary PC and Graveyard
    changes; and
17. existing battle, blackout, level-cap, capture-rule, and Healing Kit flows
    continue to work.

Codex does not run the Heartless Gold build, tests, or emulator. Build and
focused in-game verification remain user-run activities.
