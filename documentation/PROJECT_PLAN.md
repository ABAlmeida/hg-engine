# Pokémon Heartless Gold Project Plan

Last updated: 2026-07-28

This is the source-controlled status of the Heartless Gold implementation
plan. `Complete` means the feature is represented in source and has received
the verification noted below. It does not mean the overall game is content
complete.

## Current decisions

- Use expanded save data; compatibility with ordinary HeartGold saves and
  PKHeX is not a requirement.
- Award no battle experience. Defeated opposing Pokémon still award effort
  values.
- Store one monotonic level-cap value in the save. Configured trainer
  victories raise it through a central table; lower values are ignored.
- Let eligible party Pokémon advance to the saved cap through `LEVEL TO CAP`,
  preserving normal move-learning and evolution flows.
- Replace passive land and surfing encounters with Poké Bait and Shiny Bait.
- Apply capture challenge rules immediately on a new save. Track one eligible
  encounter per displayed map section, use the Pokédex for exact-species
  duplicate checks, and treat Safari Zone and Bug-Catching Contest encounters
  through their documented special rules.
- Keep Generation 5+ species work deferred until the core progression and
  encounter systems are stable.

## Status

| Phase | Status | Notes |
| --- | --- | --- |
| Repository and baseline build | Complete | Reproducible source build and historical melonDS boot baseline are recorded in `BASELINE_BUILD.md`. |
| Build performance | Complete | Message collation, dependency invalidation, host-tool optimization, timestamp-driven packaging, incremental targets, and optional `ccache` support are implemented. |
| Expanded save and PC support | Complete | `ALLOW_SAVE_CHANGES` and `EXPAND_PC_BOXES` are enabled. |
| Disable capture experience | Complete | Capture experience remains disabled. |
| No battle experience | Complete; manually verified | Battle EXP presentation and gains are skipped while eligible party members still receive effort values. |
| Persistent level caps | Complete; manually verified | New saves begin at level 7 and story-trainer victories raise the saved cap according to `LEVEL_CAPS.md`. |
| Level to Cap | Complete; manually verified | The party action advances one level at a time and preserves move and evolution prompts. |
| Bait encounters | Complete; manually verified | Poké Bait and Shiny Bait work on valid terrain; invalid use is rejected; preserved encounter types remain available. See `BAIT_ENCOUNTERS.md`. |
| Reusable healer | Implemented; build and manual verification pending | Professor Elm gives new players a reusable Healing Kit after they receive their starter. It fully restores party HP, PP, and major status outside battle, is never consumed, and cannot be used in battle. See `REUSABLE_HEALER_PLAN.md`. |
| Forced-female protagonist | Pending | Skip gender selection, write the female profile value, continue to name selection, and retain the standard Lyra graphics without replacing Ethan's unrelated NPC assets. Existing saves do not need migration. |
| HM field actions | Pending | Remove the requirement for a party Pokémon to know or be compatible with Cut, Surf, Strength, Rock Smash, Waterfall, Whirlpool, Fly, or Flash while preserving terrain checks and badge/story progression gates. |
| Evolution modernization | Pending | Convert trade and hardware/location-dependent evolutions to accessible items, lower the friendship threshold, and preserve reasonable distinctive conditions. |
| Capture challenge rules | Implemented; manual verification pending | Ordinary encounters, Safari's shared saved opportunity, duplicate enforcement, the shiny clause, and first-retained-Pokémon Contest handling are implemented. Focused Safari and revised Contest retention checks remain. See `CAPTURE_RULES_PLAN.md`. |
| Bug-Catching Contest availability | Pending after capture rules | Remove the weekday restriction so the Contest can be entered every day. Preserve other entry requirements and the existing daily participation limit unless changed separately. |
| Generation 5+ Pokémon integration | Deferred | Audit species data, assets, cries, forms, evolutions, learnsets, Pokédex/save paths, encounters, gifts, and trainer placement. Compiling a species does not place it in the game. |
| Trainer and wild content rebalance | Pending | Build teams and encounter tables around the finalized cap curve and available roster. |
| Trainer AI changes | Pending | First enable the strongest suitable existing trainer AI, then add a trainer-only fair-information decision layer, switching and item evaluation, doubles coordination, and bounded search. Wild and scripted AI must retain their original routes. |
| Graphics and presentation | Pending | Replace the temporary Bait icons, redesign rival battle/overworld graphics, add challenge messages, and consider title-screen changes after core systems stabilize. |
| Full-game regression pass | Pending | Perform milestone, save/reload, encounter, progression, and hardware/emulator checks after the remaining systems and content are integrated. |

## Level-cap milestones

The authoritative cap table, trainer IDs, save behavior, and exclusions are in
[`LEVEL_CAPS.md`](LEVEL_CAPS.md). Every requested milestone remains an
independent table entry, including Pryce, Clair, and the Kanto leaders whose
current rewards do not change the cap.

## Bait acquisition and behavior

The authoritative behavior, item acquisition replacements, and patch manifest
are in [`BAIT_ENCOUNTERS.md`](BAIT_ENCOUNTERS.md). Rare Candy remains
functional but has no normal acquisition source. Repel, Super Repel, and Max
Repel retain their IDs for table stability but have no normal field use or
acquisition source while Bait encounters are enabled.

## Capture challenge rules

The reviewed implementation design is in
[`CAPTURE_RULES_PLAN.md`](CAPTURE_RULES_PLAN.md). It records immediate
activation, exact-species Pokédex duplicates, the shiny clause, the map-section
save model, Safari Zone handling, Bug-Catching Contest retention, and the later
Contest scheduling change.

## Reusable healer

The reviewed implementation design is in
[`REUSABLE_HEALER_PLAN.md`](REUSABLE_HEALER_PLAN.md). Normal Bag use follows
the standard Key Item flow: it closes the Bag, heals through one central
operation, displays the success message in the field, and returns control to
the overworld. Registration is deliberately disabled.

## Verification policy

- Building the ROM, manually smoke-testing it, and running automated battle
  tests are distinct activities.
- Codex does not run Heartless Gold builds, tests, test scripts, or emulators;
  it provides the user with the narrow build and manual verification steps.
- Do not reuse emulator save states across code revisions when validating
  hooks or save-backed behavior; use an in-game save and reload.
- Preserve the historical baseline in `BASELINE_BUILD.md`. Record later
  feature verification here or in a dedicated feature document instead of
  rewriting that baseline.
