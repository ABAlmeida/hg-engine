# Adding Party-Use Items Safely

This guide records the integration rules learned while adding IV Max. These
items cross the Bag, party menu, a separately linked overlay, and the fixed
injected-code region; a successful link does not prove those boundaries are
safe at runtime.

## Classify the execution region

- Ordinary `src/*.c` code is linked into overlay 129 through `src/linker.ld`.
  Its code and literals consume the fixed injected-code budget.
- `src/individual/PartyMenu_HandleUseItemOnMon.c` is linked independently as
  overlay 149 at `0x023C0400` and loaded only for the party-use dispatcher.
- Assembly hooks inherit the instruction set and register context of their
  patched sites. The relevant party-menu sites are Thumb code.
- Item records, text, and icons occupy ROM archives and their loading heaps,
  rather than overlay-129 code space.

Before adding a call, record the caller region, callee region, instruction set,
lifetime, and affected memory budget.

## Cross-region call rules

When `src/individual/*.c` calls a function from ordinary injected C:

1. Declare and define the exported function with `LONG_CALL`.
2. Include that declaration at the call site; do not duplicate a prototype
   without the attribute.
3. Keep internal helpers private to their owning link unit.
4. Set bit zero on raw Thumb function pointers. In assembly, copy a known-good
   `bx` or veneer pattern.
5. Inspect the linked overlay disassembly. The call must resolve through a
   Thumb-capable absolute sequence or veneer.

IV Max follows this contract through `StatTrainingItem_UseOnMon`, which is
called from overlay 149 and implemented in overlay 129.

## Separate mutation from presentation

The party-item flow has three distinct jobs:

1. `CanUseItemOnMonInParty` validates the selected target.
2. `UseItemOnMonInParty` applies ordinary party-item mutations.
3. `PartyMenu_HandleUseItemOnMon_Internal` chooses the follow-up UI state.

A stat-changing item is not a form-change item. Do not send it into
`PARTY_MENU_STATE_FORM_CHANGE_ANIM`: that state expects animation resources and
a completion signal that a stat item does not provide. IV Max is handled before
`UseItemMonAttrChangeCheck`, applies its mutation once, prints a normal item
result message, and exits through
`PartyMenu_ItemUseFunc_WaitTextPrinterThenExit`.

The Max vitamins use the same presentation path. Their item-data EV amount is
zero because the signed archive field cannot represent 252; the central stat
handler supplies the real amount. Returning them to the vanilla vitamin
callback is unsafe because that callback expects a positive archive amount.

Consume the item only after mutation succeeds. A no-effect result must consume
nothing. Recalculate party stats once after all EV or IV fields are written.

## Assembly register safety

ARM/Thumb calls may clobber `r0` through `r3`. Reload any global address needed
after a `bl`, and keep the pointer distinct from its value:

```asm
ldr r1, =partyMenuSignal
mov r2, #0
str r2, [r1]
```

Writing through a returned object pointer instead can silently corrupt UI
data. For every hook store, state what address its base register holds at that
instruction.

## Content and static-review checklist

- Define new IDs only in `include/constants/item.h`. The build generates
  Armips-compatible definitions in `build/generated/armips_items.s`.
- Add item data, name, description, article, icon, and palette in matching ID
  order. Reused graphics still need members for the new item IDs.
- Protect base-game-only lookup tables from custom IDs.
- Assert every item-ID or Pokémon-data range used arithmetically is contiguous.
- Confirm validation and mutation enforce identical Egg, species, and cap rules.
- Confirm success consumes one item and failure consumes none.
- Confirm the chosen UI state owns every resource and completion signal it
  expects.
- Confirm individual-overlay output stays within its linker `LENGTH`, then
  remeasure overlay-129 headroom for ordinary C or assembly additions.
- Use Thumb-aware symbol breakpoints. If execution reaches `0xFFFF0104`, treat
  it as the exception handler and capture the last safe caller/callee state;
  its exception-mode stack pointer is not the interrupted user stack.

When explicitly requested, build with `make quick-rom -j$(nproc)`. Manually
check success, no effect, item consumption, displayed stats after returning,
save/reload persistence, and an existing item using the shared dispatcher.
