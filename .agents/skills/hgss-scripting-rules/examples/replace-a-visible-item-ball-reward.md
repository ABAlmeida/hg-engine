# Replace a visible item-ball reward

## Goal

Replace the item awarded by an existing visible item ball without changing
its quantity, collection flag, object behavior, or standard pickup flow.

## Target

- Script archive/source: member 141 of `a/0/1/2`, patched by
  `armips/scr_seq/scr_seq_medicine_item_replacements.s`.
- Script entries: visible item balls containing Antidote, Paralyze Heal,
  Awakening, Burn Heal, Ice Heal, EnergyPowder, Energy Root, or Heal Powder.
- Trigger or caller: each existing visible item-ball object and its existing
  collection flag. The patch does not alter object or flag identifiers.

## Verified HGSS commands

Not used. The existing scripts and standard item-ball command flow remain
unchanged; only their embedded item-ID operands are replaced.

## Verified identifiers

- Original and replacement IDs come from `asm/include/items.inc`. The current
  replacement manifest is kept next to the guarded offsets in
  `armips/scr_seq/scr_seq_medicine_item_replacements.s`.
- The mappings are Antidote to Pecha, Paralyze Heal to Cheri, Awakening to
  Chesto, Burn Heal to Rawst, Ice Heal to Aspear, Heal Powder to Lum,
  EnergyPowder to Oran, and Energy Root to Sitrus.
- Existing collection-flag names may retain the vanilla item identity. Their
  values and purpose are unchanged, so renaming them is unnecessary churn.
- Variables, messages, trainers, map IDs, event IDs, and NPC IDs: not used by
  the replacement patch.

## Minimal pattern

```asm
.if readu16("build/a012/2_141", offset) != ORIGINAL_ITEM && readu16("build/a012/2_141", offset) != REPLACEMENT_ITEM
    .error "Item-ball replacement found an unexpected item ID"
.endif

.open "build/a012/2_141", 0
.org offset
.halfword REPLACEMENT_ITEM
.close
```

The assertion accepts both the pristine and already-replaced values so the
patch remains safe when deliberately reapplied by `make rebuild_scripts`.

## Control-flow checklist

- No script branches, locks, messages, fades, or tasks are changed.
- The original item-ball quantity and collection flag remain in force.
- The assertion fails the build if an upstream archive changes the expected
  operand, avoiding a blind write to an unverified offset.

## Build and manual verification

Build from MSYS2 UCRT64 with:

```sh
make quick-rom -j$(nproc)
```

Verify that each original item ball awards the replacement Berry exactly once,
uses the replacement item's name in the standard pickup message, disappears
after collection, and remains collected after saving and reloading.
