# HM Field Actions Plan

## Intended behavior

Allow the following field actions without requiring a compatible party
Pokémon to know the corresponding move:

| Field action | Required machine |
| --- | --- |
| Cut | HM01 |
| Fly | HM02 |
| Surf | HM03 |
| Strength | HM04 |
| Whirlpool | HM05 |
| Rock Smash | HM06 |
| Waterfall | HM07 |
| Rock Climb | HM08 |
| Flash | TM70 |

The player must still have obtained the corresponding machine. Existing badge,
terrain, map, follower, costume, Safari/Pal Park, and other field-state checks
remain authoritative. Obtaining a badge without obtaining the machine does not
unlock its action.

HM and TM teaching remains available and unchanged. Pokémon move compatibility
is ignored only when deciding whether an owned machine can supply its field
action.

Flash is TM70 in HeartGold rather than an HM, but it is included because it
already participates in the same field-move system.

No new save field or migration is required. Unlock state comes from the Bag.
An existing save gains access as soon as it contains the relevant machine.

## Presentation

The existing scripts and animations require a party slot so they can display
and animate a Pokémon using the move. Removing move knowledge while retaining
that presentation means a Pokémon can visibly perform a field action that is
not in its moveset.

Retain the standard Pokémon presentation. Prefer a party member that actually
knows the move; otherwise use the selected non-Egg Pokémon, or the first
non-Egg party member for an overworld interaction. This preserves the familiar
messages, animations, and transitions and keeps the change localized.

The player does not activate these actions by selecting the machine in the
Bag. Bag use retains its normal teaching behavior. Instead:

- Cut, Surf, Strength, Rock Smash, Waterfall, Whirlpool, and Rock Climb use
  their existing obstacle or terrain interactions. For example, the player
  faces and interacts with a cuttable tree, surfable water tile, or climbable
  wall. The original prompt, badge check, animation, and field transition
  continue normally.
- Fly is selected from any non-Egg Pokémon's Party-menu context menu after the
  player owns HM02, has the Storm Badge, and satisfies the existing map and
  field-state restrictions.
- Flash is selected through the same Party-menu route while the current map is
  dark enough to permit Flash, including the existing Ruins of Alph special
  case. HeartGold does not apply a badge requirement to Flash, so owning TM70
  and being in a valid Flash context are sufficient.

The Party menu may also expose the other seven actions when the player is in
their valid context, preserving the base game's alternate field-move route.

## 1. Centralize field-action definitions

Add a small `machine_field_actions` module guarded by
`IMPLEMENT_MACHINE_FIELD_ACTIONS`. Keep one table mapping each supported move
to:

- its required HM or TM item;
- its existing field-move index; and
- whether it is part of this feature.

Expose narrow helpers rather than duplicating move/item switches:

```c
u32 MachineFieldAction_GetCount(void);
u16 MachineFieldAction_GetMove(u32 index);
BOOL MachineFieldAction_IsUsable(const SaveData *saveData, u16 moveId,
                                 const void *fieldMoveCheckData);
```

The unlock helper checks for one copy of the mapped machine directly in the
TM/HM Bag pocket without allocating item data. It does not check species
compatibility or learned moves. `MachineFieldAction_IsUsable` then calls the
existing field-move callback for badges, terrain, and other context rules.

The module's private party-user helper scans the party once. It remembers the
first non-Egg party member while preferring a member that genuinely knows the
requested move. The remembered member is used only when the move is supported
and its machine is owned. A fainted Pokémon remains eligible, matching
ordinary field-move behavior. If the party has no non-Egg Pokémon, the action
fails cleanly instead of passing the scripts an invalid slot.

Use separate named enums for the function selector, check response, and
field-move table indices. The field-move index intentionally skips 8 through
11 because those entries are Teleport, Dig, Sweet Scent, and Chatter rather
than machine actions. Add a compile-time assertion that the definition table
has exactly the nine intended entries.

## 2. Support obstacle and terrain interactions

Replace only the base `ScrCmd_GetPartySlotWithMove` implementation used by the
`CheckMoveInParty` script command:

1. read the destination variable and requested move exactly as the original
   command does;
2. search once for a real non-Egg move user while remembering the first
   non-Egg fallback;
3. if no real user exists, use that fallback only for an owned, supported
   machine action;
4. write `PARTY_SIZE` when no valid user exists; and
5. return `FALSE`, preserving normal script execution.

This retains the existing map and common scripts. Interacting with a tree,
rock, water tile, waterfall, whirlpool, or climbable wall continues through
the original prompts, badge checks, object changes, movement, and transitions.

Do not alter `ScrCmd_MonHasMove`. That command tests a specific Pokémon and may
be used by unrelated events; broadening it would make ordinary script
questions about a Pokémon's actual moves inaccurate.

The shared command was confirmed against the US HeartGold implementation:
`PARTY_SIZE` is the not-found sentinel and the function begins at `0x0204D3CC`.
The supported address and original prologue are documented beside the hook in
`hooks`; the base field-move callback dispatcher is recorded in `rom.ld`.
Disabling the feature omits the hook and retains the original behavior.

## 3. Support use from the Party menu

The custom `sub_0207B0B0` implementation currently creates field-action
buttons only from the selected Pokémon's four learned moves. Extend it without
adding all nine actions unconditionally:

1. preserve the existing learned-field-move detection and execution behavior;
2. determine which unlocked machine actions are valid in the current
   `FieldMoveCheckData`;
3. add only currently usable actions that are not already represented by a
   learned move; and
4. retain the selected non-Egg Pokémon as the presentation slot through the
   Party menu's existing execution path.

Call the original field-move check callbacks through
`FieldMove_GetMoveFunc(FIELD_MOVE_FUNC_CHECK, ...)`. This reuses the same badge
and context rules as normal field moves; do not reproduce those checks in the
new module.

The context menu has an eight-button limit. Reserve space for the existing
Summary, Level to Cap, Switch, and Item actions, then fill the remaining slots
in this deterministic order:

1. unlocked machine actions that are usable in the current context; and
2. learned field moves, in the selected Pokémon's move-slot order, excluding
   actions already added above.

Never write past `MAX_BUTTONS_IN_PARTY_MENU`. Track added move IDs with a small
fixed-size array or bit mask so a move known by the selected Pokémon and
unlocked by its machine appears only once. In the rare saturated case, a
currently usable machine action takes priority over an unrelated learned
utility action because the former is the purpose of opening the menu in that
field context.

This provides the existing menu route for Fly and Flash and keeps Cut, Surf,
Strength, Rock Smash, Waterfall, Whirlpool, and Rock Climb available through
both the Party menu and their normal overworld interactions.

## 4. Preserve the existing field-move engine

Do not replace the current `FieldMove_Check*` or `FieldMove_Use*` routines.
They already enforce the important rules:

- Cut requires the Hive Badge and a cuttable tree.
- Fly requires the Storm Badge, an allowed map, and valid follower, costume,
  Safari, and Pal Park state.
- Surf requires the Fog Badge, a surfable facing tile, and valid player,
  follower, and costume state.
- Strength requires the Plain Badge, a pushable rock, and preserves the Ice
  Path B2F exclusion.
- Rock Smash requires the Zephyr Badge and a breakable rock.
- Waterfall requires the Rising Badge, surfing state, and a waterfall tile.
- Whirlpool requires the Glacier Badge, surfing state, and a whirlpool tile.
- Rock Climb requires the Earth Badge, a climbable wall in the facing
  direction, and valid follower and costume state.
- Flash remains restricted to Flash-dark maps and the Ruins of Alph special
  chamber path.

The use callbacks continue to own task allocation, menu shutdown, scripts,
animations, map-object changes, travel selection, and cleanup. The new code
only supplies eligibility and a safe presentation slot.

## 5. Fragility and maintainability safeguards

- Keep the move-to-machine mapping in one table shared by script and menu
  paths.
- Hook the narrow party-slot lookup command, not the complete script
  dispatcher and not individual map scripts.
- Reuse the engine's field check/use callbacks instead of copying badge or
  terrain logic.
- Preserve actual learned-move behavior before applying the machine fallback.
- Guard all new behavior with one C configuration define. The hook manifest
  reads `include/config.h` directly, so it does not need a duplicate Armips
  setting.
- Use named structures and constants for the base field-move data layouts.
- Add comments at hook boundaries explaining register/argument expectations
  and why only `ScrCmd_GetPartySlotWithMove` is replaced.
- Assert the fixed mapping-table size and document the original bytes beside
  the US-specific function-entry hook. The current hook tool does not enforce
  original-byte assertions, so changing the supported base ROM still requires
  revalidating that address before building.
- Keep Teleport, Dig, Sweet Scent, Chatter, Headbutt, Milk Drink, and
  Softboiled outside the fallback table.

The runtime work is a few Bag and party checks during a menu opening or a
scripted field interaction. It is not part of the per-frame overworld loop and
has no meaningful gameplay-performance cost.

## 6. Implementation sequence

Implement this as three reviewable changes:

1. add the configuration flag, central mapping/helpers, declarations, and
   linker symbols;
2. hook `ScrCmd_GetPartySlotWithMove` for direct field interactions; and
3. extend Party-menu population with deduplication and strict capacity
   handling.

After each change, perform a static review for feature-disabled behavior,
fixed-address assumptions, invalid party slots, duplicate menu entries, menu
capacity, memory ownership, and consistency between the direct-interaction and
Party-menu paths.

## 7. Build and manual verification

After implementation, the user creates an incremental ROM from the MSYS2
UCRT64 shell:

```sh
make quick-rom -j$(nproc)
```

Codex must not run the Heartless Gold build, test suite, or emulator.

Use an in-game save and reload it after replacing the ROM; do not reuse a save
state across code revisions. Manually verify:

1. each action is unavailable before obtaining its HM or TM;
2. owning each machine enables its action without teaching the move;
3. no compatibility check is applied to the presentation Pokémon;
4. a real move user is preferred when one exists;
5. Cut, Surf, Strength, Rock Smash, Waterfall, Whirlpool, and Rock Climb work
   through direct overworld interaction;
6. all nine actions appear through the Party-menu route when their normal
   context permits;
7. every original badge requirement remains enforced;
8. invalid terrain, maps, follower state, Rocket costume, Safari/Pal Park
   state, and already-surfing state retain their original rejection behavior;
9. Fly opens the town map, permits only valid destinations, and completes the
   transition;
10. Flash works in a Flash-dark map and in its Ruins of Alph special case;
11. the scripts display and animate the intended Pokémon under the chosen
    presentation design;
12. a party with no valid non-Egg presentation Pokémon fails safely;
13. learned field moves continue to work when the machine fallback is not
    applicable;
14. no duplicate field-action button appears;
15. a Pokémon with several learned utility moves cannot overflow or corrupt
    the eight-button context menu;
16. HM/TM teaching behavior is unchanged;
17. all non-target field moves retain their original behavior;
18. saving and reloading does not change availability beyond the Bag contents;
    and
19. disabling `IMPLEMENT_MACHINE_FIELD_ACTIONS` restores the original
    move-knowledge requirement.

Implementation is complete. An incremental ROM build and focused manual
verification remain pending.
