---
name: maintain-trainer-reward-trackers
description: Keep Pokemon Heartless Gold trainer reward source data and the TM, vitamin, Berry, and held-item availability Markdown trackers synchronized. Use whenever data/trainer_rewards.csv or another first-victory trainer reward is added, removed, reassigned, renamed, re-quantified, or manually verified.
---

# Maintain Trainer Reward Trackers

The repository `AGENTS.md` remains authoritative. This source/documentation
workflow grants no build, test, ROM-inspection, or emulator authorization.

## Required workflow

Whenever trainer rewards change:

1. Read the complete `data/trainer_rewards.csv` and the code or script path for
   any reward that bypasses that table.
2. Resolve every numeric trainer and item ID from authoritative project source.
   Never infer an item from an incomplete nearby comment.
3. Update the reward-table comment to include the trainer, location, and item
   name. Comments are audit aids; the numeric row remains authoritative.
4. Classify the item, then read and update only the applicable trackers:
   - TM: `documentation/TM_REWARD_AVAILABILITY.md`
   - vitamin: `documentation/VITAMIN_AVAILABILITY.md`
   - Berry: `documentation/BERRY_REWARD_AVAILABILITY.md`
   - other functional held item: `documentation/HELD_ITEM_REWARD_AVAILABILITY.md`
   Update more than one only when the item genuinely belongs to more than one
   category. Never edit the marked generated unusable-TM section by hand.
5. In each affected tracker, update the assignment, source, quantity,
   availability, verification, dependent summary counts or finite totals, and
   `Last updated` date.
6. Keep a new or changed reward at `Not verified` until the user reports that
   it was successfully received in a current build. Do not infer verification
   from source inspection or a successful build.
7. Check related summaries in `documentation/PROJECT_PLAN.md` and feature plans
   when the change alters planned or completed progression.
8. Update the affected `documentation/POKEMON_TRAINERS.md` row when that
   trainer is documented there. Party, IV, AI, and held-item changes remain
   governed by the Pokémon-availability workflow.

## Classification rules

- Classify TM items by their item-to-move mapping, not merely an `ITEM_TM`
  name. Keep one TM tracker row per distinct move.
- Classify all 24 S, regular, L, and Max stat vitamins in the vitamin tracker.
  Recalculate finite EV totals using the documented tier values.
- Classify only standard Berries recognized by `IS_ITEM_BERRY` in the Berry
  tracker unless the project deliberately expands that scope.
- Classify a non-Berry as a held-item candidate only when its item-data
  `holdEffect` is neither `0` nor `SOUBI_NONE`. Note species-specific,
  form-changing, evolutionary, or progression-sensitive caveats.
- If an item fits more than one functional category, record the reward in each
  relevant tracker and keep the category summaries internally consistent.

## Review and validation

- Compare every documented assignment with the numeric CSV row and item
  constant.
- Search the affected trackers and trainer row for stale names, quantities,
  availability labels, totals, and summary counts.
- Run `git diff --check` and review the full resulting diff.
- Preserve unrelated working-tree changes.
- Report documentation-only verification accurately; do not claim runtime
  reward behavior was tested.
