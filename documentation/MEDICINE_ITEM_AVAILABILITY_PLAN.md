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

## Future implementation

1. Audit marts, visible and hidden pickups, gifts, prizes, trades, scripts,
   held items, and other source-controlled reward tables for every listed ID.
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
