# Medicine Item Availability Cleanup Plan

Last updated: 2026-08-02

## Scope

Keep the following item IDs and item-data records intact, but eventually remove
all of their normal acquisition sources:

- Revive
- Max Revive
- Revival Herb
- Sacred Ash
- Antidote
- Burn Heal
- Ice Heal
- Awakening
- Paralyze Heal
- EnergyPowder
- Energy Root
- Heal Powder
- Lava Cookie
- Old Gateau

This is a separate content pass. It is not required to add the stat-training
items, and no sources are removed by the stat-training implementation.

## Implemented replacements

- EnergyPowder acquisition sources now award Oran Berries instead.
- Energy Root acquisition sources now award Sitrus Berries instead.
- Antidote acquisition sources now award Pecha Berries instead.
- Paralyze Heal acquisition sources now award Cheri Berries instead.
- Awakening acquisition sources now award Chesto Berries instead.
- Burn Heal acquisition sources now award Rawst Berries instead.
- Ice Heal acquisition sources now award Aspear Berries instead.
- Heal Powder acquisition sources now award Lum Berries instead.
- The Goldenrod Underground herbal shop is now the Goldenrod Berry Shop and
  stocks every Berry recognized by the engine. Berry base prices are globally
  ten times their previous values: the former $20 Berries cost $200 and the
  former $80 Berries cost $800. The shop does not use a separate price override.

The replacement pass covers normal marts, department-store inventories, hidden
items, visible item balls, and Pickup's repeatable Antidote reward. Converting
the herbal shop also removes Revival Herb from that shop. No normal acquisition
source for Lava Cookie or Old Gateau was found in the source-controlled content,
so they remain unobtainable without altering their item records. The original
medicine IDs and item-data records remain intact for compatibility with existing
saves.

## Future implementation

1. Audit marts, visible and hidden pickups, gifts, prizes, trades, scripts,
   held items, and other source-controlled reward tables for Revive, Max Revive,
   Revival Herb, and Sacred Ash.
2. Classify each source as removal or replacement. Decide replacement items
   individually; no replacement mapping is assumed in advance.
3. Preserve event progression when a gift or pickup is changed. An empty
   interactable object or a skipped state-setting command is not acceptable.
4. Keep existing saves valid. Previously obtained copies may remain harmlessly
   in the Bag even though new copies cannot be obtained.
5. Recount the reachable Medicine roster after the audit. Expand the pocket
   only if later content can genuinely require more than 40 distinct Medicine
   stacks at once.

## Verification

- Search every source-controlled acquisition surface for all 14 item IDs.
- Exercise altered gifts and pickups to confirm their event flags still work.
- Confirm none of the items can be newly obtained in a complete progression
  pass while old saves containing them continue to load normally.
