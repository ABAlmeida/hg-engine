# Heartless Gold Code Budget

This document tracks runtime memory costs for Heartless Gold features. Update
it whenever a change adds or removes injected C/assembly, save fields, dynamic
allocations, or assets with meaningful heap or VRAM requirements.

## How to read the measurements

The synthetic overlay linked through `src/linker.ld` has a fixed capacity of
32,672 bytes (`0x7FA0`). Code, read-only data, initialized data, and BSS in the
linked top-level `src/*.c` and `asm/*.s` objects all consume this same region.

The last successful link before the Summary friendship work used 32,559 bytes
and left 113 bytes free. The first friendship implementation increased
`summary.o` by 120 bytes and therefore overflowed the region by 7 bytes. The
refactored implementation calculates nature once per Summary refresh and
replaces the 150-byte nature-effect table with the equivalent 5-by-5 nature
calculation. Its final object is 1,144 bytes, 46 bytes larger than the
pre-friendship object.

Challenge capture and permanent-death code was subsequently split by runtime
lifetime. The first 2026-08-07 build reduced overlay-129 allocation to 31,295
bytes, but manual startup testing proved that moving direct ARM9 hook entries
into overlay 131 was unsafe. Permanent-death handling is resident again, while
capture code remains safely divided between its field, battle, and shared-state
lifetimes. The corrected 2026-08-07 build uses 31,699 of 32,672 bytes, leaving
973 bytes free and safely reclaiming 912 bytes from the 32,611-byte baseline.

Object totals below are **gross current object sizes**, not always the
incremental cost of a Heartless Gold feature. Files inherited from hg-engine or
shared by several features are labelled accordingly. Function sizes are useful
for locating expensive subfeatures, but alignment, literal pools, shared
helpers, and linker padding mean they cannot always be added into an exact
feature total.

## Injected overlay feature costs

| Feature | Object or component | Measured bytes | Notes and possible savings |
| --- | --- | ---: | --- |
| Level caps | `level_cap.o` | 188 | `SetNewLevelCap`: 48; victory-table application: 76; battle-finish hook: 60. The trainer/cap table is packaged data and does not consume this region. Already compact. |
| Poké Bait and Shiny Bait | `bait.o` | 377 | Tile/use validation: 168; item-use task: 52; menu use: 116; mode queries: 32; state: 1 plus alignment. A shared encounter task avoids duplicated normal/shiny implementations. |
| Challenge capture rules | `capture_rules.o` | 53 | Overlay 129 now retains only initialization and the one-byte permission that must survive the field-to-battle transition. The relocated field component is 632 bytes and the battle component is 324 bytes. This reclaimed 876 directly attributable overlay-129 bytes. |
| HM use without teaching | `machine_field_actions.o` | 474 | Script lookup/use command: 248; usability check: 84; ownership check: 56; move table: 54; accessors: 28. Table-driven design is already maintainable; optimize only with measurements. |
| Permanent death and wipe handling | `permanent_death.o` | 957 | All state, party retirement/recovery, notification scheduling, deferred callbacks, and direct ARM9 hook entries remain resident. The independently linked field extension cannot safely supply unresolved or early-lifetime ARM9 targets. The rejected split's 554-byte root measurement is retained only in the historical snapshot below. |
| Reusable Healing Kit | `reusable_healer.o` | 316 | Healing: 104; field task: 132; menu/field entrypoints: 72; alignment: remainder. Small and centralized. |
| Stat-training items | `stat_training_items.o` | 540 | Effect lookup: 116; validation: 172; application: 212; handled-item check: 32; alignment: remainder. Shared vitamin handling avoids per-item functions. |
| Summary stat/IV/EV viewer, nature colours, and friendship | `summary.o` | 1,144 | The final object is 46 bytes larger than the 1,098-byte pre-friendship object and 74 bytes smaller than the failed first implementation. It shares the Pokémon pointer and nature result and calculates nature effects instead of storing the former 150-byte table. |
| Goldenrod Berry Shop inventory | `mart.o` data table | Measurement pending; nominal payload grows by 126 bytes | The former five-entry, 10-byte herbal table becomes a 68-entry, 136-byte Berry table including its terminator. This uses the existing mart path and avoids a larger custom shop/price hook; confirm the linked delta and remaining headroom on the next explicitly requested build. |
| General script commands | `script_new_cmds.o` | Measurement pending | Previously 136 bytes. Configured Egg IV/ability handling now shares this dispatcher; measure its new object size and remaining overlay-129 headroom on the next explicitly requested build. |

## Injected features within shared objects

These measurements are sums of named functions in the current objects. They
exclude shared helpers, literal pools, alignment, and hooks that cannot be
attributed safely, so they are lower bounds rather than exact totals.

| Feature | Measured subfeatures | Attributable bytes | Notes |
| --- | --- | ---: | --- |
| Level to Cap | Party-menu state/flow: 656; Pokémon one-level and capped-level routines: 280 | At least 936 | This is one of the largest Heartless Gold additions. The staged one-level flow is necessary for move learning and evolution, but the party-menu state machine is a future optimization candidate. |
| Instant Egg Hatch | Start script: 72; custom menu action: 140; return handler: 192 | At least 404 | Shared context-menu rendering and dispatch changes are excluded. Review whether the return handler can reuse more of the vanilla menu re-entry path before adding more Egg actions. |
| Stat-training item party-menu integration | Patched item-use handler object: 2,764 gross | Unattributed | `PartyMenu_HandleUseItemOnMon_linked.o` contains the modified vanilla routine, so its entire size is not the feature cost. Measure against the same routine without stat-training support for an exact delta. |
| Expanded save/PC support | `save.o`: 3,033; `pokemon_storage_system.o`: 2,152 gross | Unattributed | Foundational shared code required by several features. Exact incremental cost needs a configuration-off comparison; save correctness takes priority over byte reduction. |
| No battle EXP and no battle EVs | Modified battle-script command object | Unattributed | Implemented inside a large linked battle routine rather than a dedicated object. It should be measured with controlled configuration-on/off builds. |
| Trainer-battle active-item restriction | Modified player battle-controller object | Unattributed | Implemented inside a large linked overlay routine. Held items are unaffected. Measure as a conditional build delta rather than charging the full object. |
| Debug item/Rayquaza seeding | `bag.o` and `pokemon_storage_system.o` conditional blocks | Currently disabled | It contributes no active build cost while `DEBUG_CHEATS` is undefined. Measure only when temporarily enabled. |

## Large shared objects in the injected region

These objects are major consumers, but their gross totals must not be assigned
wholly to one Heartless Gold feature without a historical comparison.

| Object | Measured bytes | Main contributors or review direction |
| --- | ---: | --- |
| `pokemon.o` | 8,242 | Broad hg-engine Pokémon/form/evolution support plus level-to-cap functions. `Pokemon_LevelToCapOneLevel` is 116 bytes and `Pokemon_TryLevelUp` is 164 bytes; most of the object is shared engine functionality. |
| `party_menu.o` | 3,267 | Shared party menu, level-to-cap UI, Egg Hatch action, reusable-item flow, and form actions. Large inherited functions dominate; isolate changes by comparing against the introducing commit before optimizing. |
| `save.o` | 3,033 | Expanded-save implementation and generic save routines. Treat as shared infrastructure; do not remove validation or asynchronous-write handling merely to save bytes. |
| `item.o` | 2,312 | Includes a 456-byte field-use function table and a 680-byte machine-move table, plus shared item functions. Tables are the largest components. |
| `pokemon_storage_system.o` | 2,152 | General expanded-PC implementation. This is shared save/storage infrastructure rather than the removed Graveyard design. |
| `bag.o` | 1,980 | General Bag implementation, registration changes, machine labels, sorting, and field helpers. Attribute savings to individual changes only through commit-to-commit measurement. |
| `other_hook.o` | 1,540 | Shared assembly trampolines for many unrelated systems. Most symbols lack reliable ELF size metadata, so audit hook ranges from source and linked disassembly before attributing bytes. |
| `summary.o` with failed friendship version | 1,218 | Includes 1,068 bytes of code and 150 bytes of nature data. This is the artifact from the failed link, not an accepted budget baseline. |
| `capture_rules.o` | 53 | Lifetime-critical capture state only. The field and battle components are tracked separately above and do not consume overlay 129. |

## Features stored outside the injected overlay

| Feature/content | Primary budget | Injected bytes | Notes |
| --- | --- | ---: | --- |
| Trainer level-cap mapping | ROM data/NARC | 0 table bytes | `TrainerLevelCaps.o` is converted to packaged data; its current payload is 498 bytes. Lookup code remains in `level_cap.o`. |
| Trainer victory rewards | Field scripts, text NARC, generated trainer data | No dedicated C object | Reward mappings and dialogue do not occupy the synthetic overlay. Existing shared script command infrastructure may still be used. |
| Talk-initiated trainer battles | Field scripts and Armips patches | 0 synthetic-overlay bytes for script content | Direct binary patches consume/reuse bytes in their owning executable region; track their exact patched ranges separately if expanded. |
| Revised opening sequence and gifts | Field scripts and text NARC | 0 | Dialogue and script commands increase archive size, not injected-code usage. |
| Reusable configured Egg gifts | Field scripts, text NARC, and shared script-command code | Pending measurement | Standard script 2075 centralizes party-space checking, Egg creation, fixed-IV/ability configuration, and completion state. The menu and messages remain map-local; the compact runtime configurator consumes overlay-129 bytes that must be measured on the next requested build. |
| Wild/trainer roster edits | Encounter/trainer NARCs | 0 | Data-only changes affect ROM archive size and runtime archive loads, not the fixed synthetic overlay. |
| New item names/descriptions/icons | Item/message/graphics NARCs | 0 for the assets | Item behavior code is accounted for in `item.o`, `bait.o`, `reusable_healer.o`, and `stat_training_items.o`. Graphics also require heap/VRAM review when new assets are introduced. |
| Configuration-only behavior | Existing code paths | Usually 0 incremental | Examples include fast text, National Dex configuration, and disabling trainer-battle item use when the implementation already exists upstream. Confirm with a link comparison if a conditional compiles additional code. |
| Instant text and always-available National Dex | In-place/configured engine behavior | No dedicated synthetic-overlay object | Their patches belong to the original executable regions rather than the fixed synthetic overlay. |
| Disable trainer line of sight | In-place ARM9 patch | 0 synthetic-overlay bytes | The patch replaces original instructions and does not allocate from overlay 129. Keep its address/range documented because its cost is fragility rather than overlay space. |
| HM, opening, Bait, Healing Kit, reward, and medicine acquisition edits | Scripts, data tables, and in-place patches | 0 synthetic-overlay bytes | Their runtime behavior may call injected functions already accounted for above. Medicine replacements alter packaged tables or overwrite existing two-byte item IDs rather than adding linked code. |

## Planned features

Planned work is listed here so code-budget impact is considered before
implementation rather than after the linker is full.

| Planned feature | Expected budget | Current guidance |
| --- | --- | --- |
| Laptop PC access | Injected field/item entry code plus existing PC overlay | Prefer the existing PC launcher and one narrow field task. Measure before adding custom UI or duplicate PC state. |
| Smarter trainer switching AI | Prefer reclaimed/in-place overlay 10 code | Extend the existing switch evaluator and AI modules. Avoid a large new overlay-129 decision engine. |
| Forced-female protagonist | Script/in-place configuration | Should not require synthetic-overlay code if implemented through the existing intro flow. |
| Bug-Catching Contest daily availability | Field scripts/data | Prefer changing the verified schedule checks without new C code. |
| Battle Item acquisition removal | Content data/scripts | No synthetic-overlay code expected. Keep item IDs stable. |
| Graphics and presentation | ROM, heap, and VRAM | Track compressed asset size and loaded runtime footprint separately from executable code. |

## Runtime and persistent-memory costs

| Feature | Memory class | Cost/status |
| --- | --- | --- |
| Level cap and capture tracking | Expanded save data | Stored in the expanded save structures. Record exact field and enclosing-structure deltas when those layouts next change. |
| Bait encounter state | BSS | 1 byte in `bait.o`, plus linker alignment. |
| Capture permission state | BSS | 1 byte in `capture_rules.o`, plus linker alignment. Persistent capture history belongs to save data rather than BSS. |
| Permanent-death notifications | BSS/data | Two bytes remain in overlay 129 for cross-lifetime notification/recovery state. The field extension owns its four-byte active-task pointer. Party changes themselves reuse existing party/PC storage. |
| Summary friendship display | Heap/stack/save | No new save state or allocation. It reads the existing friendship field during Summary rendering. |
| Text and script features | ROM archive and load heap | Archive growth should be measured independently when it becomes material; it does not reduce synthetic-overlay headroom. |

## Required update procedure

After a user-requested successful code or ROM build:

1. Record total linked allocation and remaining bytes in the snapshot below.
2. Compare affected object section sizes with their previous committed build.
3. Record incremental bytes for each feature and meaningful subfeature.
4. Classify non-code changes separately as save, heap, stack, archive, or VRAM.
5. Flag any optimization that depends on hard-coded addresses or discarded
   validation so its fragility is visible.

### Link snapshots

| Snapshot | Used | Capacity | Free | Result |
| --- | ---: | ---: | ---: | --- |
| Before Summary friendship | 32,559 | 32,672 | 113 | Successful historical build |
| First Summary friendship implementation | 32,679 | 32,672 | -7 | Link failed |
| Shared-nature/formula friendship implementation, before feature relocation | 32,611 | 32,672 | 61 | Successful build artifact used as relocation baseline |
| Unsafe first capture/permanent-death lifetime split | 31,295 | 32,672 | 1,377 | Built on 2026-08-07, but startup testing failed because ARM9 hook targets were placed in an extension that was not loaded yet. Historical measurement only. |
| Safe capture-only lifetime split | 31,699 | 32,672 | 973 | Successful `quick-rom` build on 2026-08-07. Permanent-death code remains resident; capture field and battle logic remain relocated. |

### Extension-overlay impact of the safe split

| Region | Before | After | Increase | Capacity |
| --- | ---: | ---: | ---: | ---: |
| Battle extension (overlay 130) | 71,868 | 72,192 | 324 | 81,920 |
| Field extension (overlay 131) | 20,230 | 20,870 | 640 | 98,304 |

The safe extension increase is 964 bytes. It consists of the ordinary and
Bug-Contest capture paths in the field extension and Safari initialization in
the battle extension. Both extensions retain substantial headroom and are
entered only from code whose owning overlay is loaded. Permanent-death code is
not included in either extension.
