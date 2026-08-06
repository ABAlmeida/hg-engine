# Stat-Training Items Plan

Last updated: 2026-08-02

## Decisions

- Keep the Medicine pocket at its current 40-slot capacity. A full pocket uses
  the game's existing Bag-full behavior; no existing Medicine needs to be
  removed before these items are introduced.
- Add 18 S, L, and Max vitamin variants for HP Up, Protein, Iron, Carbos,
  Calcium, and Zinc, plus the Silver 1 reward IV Max. The vitamin variants have
  no normal gameplay source yet; `DEBUG_CHEATS` may seed them for testing and
  enables badge-independent obedience for its level-100 Rayquaza.
- Retain the six existing vitamin IDs as the unsuffixed middle tier.
- Use the following EV amounts:
  - `S`: 10 EVs.
  - Unsuffixed: 50 EVs.
  - `L`: 100 EVs.
  - `Max`: as many EVs as will fit, up to 252 in the selected stat and 510
    across all six stats.
- Never redistribute EVs. If fewer EVs fit than the nominal amount, apply the
  remaining legal amount; if none fit, report that the item has no effect.
- Preserve the normal vitamin friendship and mood increases for every EV tier.
- Make IV Max set all six real IVs to 31. It has no effect when all six are
  already 31 and does not change friendship.
- Reject Eggs. HP Up variants also retain Shedinja's normal HP-item exclusion.
- Reuse each original vitamin icon for its three variants. Use the existing
  Gold Bottle Cap icon for IV Max until bespoke art is requested.
- Award one IV Max alongside Silver 1's Oran Berry. Keep every EV-tier variant
  unobtainable until its later acquisition and balance pass.

## Implementation

1. Append stable item IDs after the existing custom items and mirror them in
   the C and assembly item constants.
2. Add item-data, name, description, article, plural-display, icon, and palette
   entries in matching ID order.
3. Change the six original vitamin records from 10 EVs to 50 EVs.
4. Route all 24 vitamin IDs and IV Max through one central party-item handler.
   Reuse the existing party item-use hooks rather than adding another binary
   hook or hard-coded address.
5. Clamp EV additions against both the 252 per-stat cap and 510 total cap,
   recalculate battle stats once, and apply native vitamin friendship rules.
6. Add IV Max to Silver 1's existing scripted reward. Leave vitamin-tier
   acquisition and wider stat-item balance for a later content pass.

See `guides/ADDING_PARTY_USE_ITEMS.md` for the party-menu, separately linked
overlay, and Thumb-call requirements discovered while implementing IV Max.

## Memory and performance

- Item records, text, and graphics consume ROM archive space, not the fixed
  overlay-129 injected-code region. The 19 item records add 684 bytes to the
  fully loaded item-data table (`19 * sizeof(ITEMDATA)`) on heaps that load it.
- Reused graphics are packaged as additional NARC members and add ROM/archive
  space; only one selected item's icon is loaded by the normal UI at a time.
- The central handler adds a small amount of injected code but no permanent
  global allocation, save data, recursion, or new hook.
- EV/IV work runs only when the player checks or uses one of these items and
  loops over exactly six stats, so it is not a field or battle hot path.

## Verification

- With `DEBUG_CHEATS` enabled and a new save, confirm all 19 items display the
  intended name, description, pocket, and reused icon.
- Confirm winning Silver 1 awards exactly one Oran Berry and one IV Max while
  losing awards neither and the battle cannot be repeated for another copy.
- Check 10, 50, and 100 point gains from zero and near both EV caps.
- Check Max at 0, 251, and 252 in the target stat and at 509/510 total EVs.
- Check Shedinja HP items, Eggs, and no-effect cases do not consume the item.
- Check friendship changes at the low, middle, and high friendship tiers.
- Check IV Max updates all six IVs, recalculates stats, preserves current
  damage correctly, and is not consumed when every IV is already 31.
- Fill all 40 Medicine slots and confirm another Medicine uses the standard
  Bag-full refusal without corrupting or losing an existing slot.
