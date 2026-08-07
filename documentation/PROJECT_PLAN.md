# Pokémon Heartless Gold Project Plan

Last updated: 2026-08-07

This is the source-controlled status of the Heartless Gold implementation
plan. `Complete` means the feature is represented in source and has received
the verification noted below. It does not mean the overall game is content
complete.

## Current decisions

- Use expanded save data; compatibility with ordinary HeartGold saves and
  PKHeX is not a requirement.
- Award no battle experience or effort values. Vitamins are the only way to
  add effort values; EV-reducing items retain their normal behavior.
- Add four vitamin strengths for each stat (10, 50, 100, and up to the legal
  maximum) plus IV Max. Silver 1 awards one IV Max; keep the vitamin variants
  without normal gameplay sources for now and retain the current 40-slot
  Medicine pocket and standard Bag-full behavior. `DEBUG_CHEATS` may seed
  temporary testing copies.
- Store one monotonic level-cap value in the save. Configured trainer
  victories raise it through a central table; lower values are ignored.
- Let eligible party Pokémon advance to the saved cap through `LEVEL TO CAP`,
  preserving normal move-learning and evolution flows.
- Replace passive land and surfing encounters with Poké Bait and Shiny Bait.
- Apply capture challenge rules immediately on a new save. Track one eligible
  encounter per displayed map section, use the Pokédex for exact-species
  duplicate checks, and treat Safari Zone and Bug-Catching Contest encounters
  through their documented special rules.
- Permanently remove fainted party Pokémon after eligible battles. On a wipe,
  recover the first usable boxed Pokémon and name it in the recovery message.
  Keep all 30 expanded PC boxes available for ordinary storage.
- Provide portable access to the shared PC menu through a reusable Laptop Key
  Item. Mum initially gives it during the existing Pokégear sequence.
- End a run with a dedicated defeat message and return to the title screen
  when a wiped player has no usable Pokémon in ordinary PC storage.
- Treat hg-engine's existing Generation 5-6 species engine and data foundation
  as supplied. Keep encounter, trainer, gift, and balance placement deferred
  until the core progression and encounter systems are stable.
- Require the player to talk to field trainers to begin battle. Trainers must
  not interrupt movement or initiate battles through line of sight.
- Give a deterministic item reward only for the first victory over a trainer
  ID, according to a source-controlled table stored in ROM data rather than
  linked C data. Rematch IDs and other trainers can be configured with no
  reward.
- Show each non-Egg Pokémon's exact friendship value on the Summary Skills
  page. Eggs retain the normal heading because their friendship field stores
  remaining hatch cycles.
- Disable active player item use in trainer battles through hg-engine's
  existing `DISABLE_ITEMS_IN_TRAINER_BATTLE` behavior. Item selections are
  rejected and return to command selection. Wild-battle Bag behavior and held
  items remain unchanged for initial playtesting.
- Remove normal acquisition sources for every item in the Battle Items pocket.
  Retain their IDs and item data for archive stability.
- Consolidate the opening so Mum supplies the early travel features, Silver 1
  occurs outside Elm's lab, the counterpart teaches Bait without a simulated
  battle, and Mr. Pokemon gives the hatchable Egg before progression continues
  directly toward Violet City. See `OPENING_SEQUENCE_PLAN.md`.
- Replace Violet City's Bellsprout-for-Onix NPC trade with a one-time choice
  between Dark, Steel, and Ice Eggs: Sandile, Tinkatink, or Swinub. Implement
  the atomic party-space check, Egg gift, IV/ability configuration, and
  completion flag as a reusable common-script helper while keeping each NPC's
  menu and dialogue map-local. Each Egg has 25 in all six IVs; Sandile has
  Moxie, Tinkatink has Mold Breaker, and Swinub has Thick Fat.

## Status

| Phase | Status | Notes |
| --- | --- | --- |
| Repository and baseline build | Complete | Reproducible source build and historical melonDS boot baseline are recorded in `BASELINE_BUILD.md`. |
| Build performance | Complete | Message collation, dependency invalidation, host-tool optimization, timestamp-driven packaging, incremental targets, and optional `ccache` support are implemented. |
| Expanded save and PC support | Complete | `ALLOW_SAVE_CHANGES` and `EXPAND_PC_BOXES` are enabled. |
| Disable capture experience | Complete | Capture experience remains disabled. |
| No battle experience | Complete; manually verified | Battle EXP presentation and gains are skipped. |
| Item-only effort values | Implemented; build and manual verification pending | `DISABLE_BATTLE_EV_GAIN` prevents defeated Pokémon from awarding EVs while vitamins remain available and can reach the modern 252-per-stat cap. EV-reducing items remain unchanged. |
| Stat-training item tiers | Implemented; build and manual verification pending | Add S, L, and Max variants for all six vitamins, make the existing unsuffixed vitamins grant 50 EVs, and add IV Max. Silver 1 awards one IV Max; vitamin-tier acquisition is deferred. See `STAT_TRAINING_ITEMS_PLAN.md`. |
| Medicine item availability cleanup | Planned; separate content pass | Keep 14 obsolete healing/status item IDs intact but later remove their normal acquisition sources. Replacement rewards remain undecided. See `MEDICINE_ITEM_AVAILABILITY_PLAN.md`. |
| Persistent level caps | Complete; manually verified | New saves begin at level 7 and story-trainer victories raise the saved cap according to `LEVEL_CAPS.md`. |
| Level to Cap | Complete; manually verified | The party action advances one level at a time and preserves move and evolution prompts. |
| Bait encounters | Complete; manually verified | Poké Bait and Shiny Bait work on valid terrain; invalid use is rejected; preserved encounter types remain available. See `BAIT_ENCOUNTERS.md`. |
| Reusable healer | Complete; Bag use manually verified; registered use pending | Professor Elm gives new players a reusable Healing Kit after they receive their starter. It fully restores party HP, PP, and major status outside battle, is never consumed, cannot be used in battle, and can be registered to a field-use button. See `REUSABLE_HEALER_PLAN.md`. |
| Revised opening sequence | Implemented; initial manual pass complete; cleanup verification pending | Consolidate Mum's early unlocks, preserve Elm's Healing Kit, give the starting supplies together, move Silver 1 onto a valid New Bark path, shorten the counterpart's tutorial to explain Bait without a battle, move the hatchable Egg to Mr. Pokémon, have Oak add an Old Rod, replace the later Egg gift with Shiny Bait, and remove the return-to-Elm requirement. See `OPENING_SEQUENCE_PLAN.md`. |
| Permanent death and wipe recovery | Complete; manually verified | Eligible fainted party Pokémon are deleted centrally after battle. Common field scripts report deaths and transactional reserve recovery after the overworld is visible; no-reserve wipes show the ending message and return to title. See `PERMANENT_DEATH_PLAN.md`. |
| Laptop PC access | Planned; independent from permanent death | Add a reusable Laptop Key Item that opens the shared PC menu from safe field contexts. Mum initially gives it during the existing Pokégear sequence. See `LAPTOP_PLAN.md`. |
| Forced-female protagonist | Pending | Skip gender selection, write the female profile value, continue to name selection, and retain the standard Lyra graphics without replacing Ethan's unrelated NPC assets. Existing saves do not need migration. |
| HM field actions | Implemented; build and manual verification pending | Owned machines enable Cut, Surf, Strength, Rock Smash, Waterfall, Whirlpool, Rock Climb, Fly, or Flash without teaching or compatibility requirements while preserving the original field checks and Pokémon presentation. See `HM_FIELD_ACTIONS_PLAN.md`. |
| Evolution modernization | Partially complete; upstream mechanics present | hg-engine already uses the modern friendship threshold of 160, provides Linking Cord routes for the original trade evolutions and held-item trades, and replaces the magnetic-field, moss-rock, and ice-rock evolutions with stones. Player-facing completion still requires normal acquisition sources for the Linking Cord and required held items, plus an audit of unsupported special methods such as Karrablast/Shelmet's paired trade. |
| Capture challenge rules | Implemented; manual verification pending | Ordinary encounters, Safari's shared saved opportunity, duplicate enforcement, the shiny clause, and first-retained-Pokémon Contest handling are implemented. Focused Safari and revised Contest retention checks remain. See `CAPTURE_RULES_PLAN.md`. |
| Bug-Catching Contest availability | Pending after capture rules | Remove the weekday restriction so the Contest can be entered every day. Preserve other entry requirements and the existing daily participation limit unless changed separately. |
| Generation 5-6 species engine/data foundation | Complete; supplied by hg-engine | The repository contains the expanded species IDs, personal data, evolutions, learnsets, experience data, battle and follower graphics, icons, cries, forms, and expanded Pokédex tables. This records the upstream foundation as complete for project planning; it is not a claim that every species and form has received focused in-game verification. |
| Generation 5+ Pokémon content integration | Deferred | Select and place the expanded roster in encounters, trainers, gifts, and other acquisition sources, then balance it against progression. The current encounter and trainer content does not make the compiled species available automatically. |
| Reusable configured Egg gifts | Implemented; build and manual verification pending | Violet City's existing trade now offers Dark/Sandile, Steel/Tinkatink, and Ice/Swinub Eggs with 25 in every IV and Moxie, Mold Breaker, or Thick Fat respectively. Standard script 2075 accepts species, met-location, completion-flag, and packed IV/ability variables, refuses a full party without consuming the gift, and sets the flag only after successfully configuring the Egg. |
| Talk-initiated trainer battles | Implemented; build and manual verification pending | `DISABLE_TRAINER_LINE_OF_SIGHT` prevents automatic trainer detection before it takes field control. The shared undefeated talk path displays a generated species/reward offer and asks whether to battle. Custom map scripts, automatic battles, defeated dialogue, and rematches remain unchanged. |
| Trainer victory rewards | Shared-trainer infrastructure implemented; content and manual verification pending | `data/trainer_rewards.csv` configures one first-victory item and quantity per shared talk trainer. The existing defeated path prevents the initial reward from repeating; current rematches and custom or automatic battle scripts do not use this table. Reward mappings will be added during trainer-content work. |
| Summary friendship value | Implemented; build and manual verification pending | The Skills page displays the exact `0-255` friendship value for party and boxed Pokémon without adding save state or another Summary hook. Eggs retain the normal heading because the same field stores hatch cycles. |
| In-battle player item restriction | Implemented; build and manual verification pending | `DISABLE_ITEMS_IN_TRAINER_BATTLE` rejects active player item selections in trainer battles and returns to command selection. Wild-battle Bag behavior and held items remain unchanged; revisit stricter UI or pocket filtering only if playtesting requires it. |
| Battle Item acquisition removal | Planned | Remove or replace marts, visible and hidden pickups, gifts, prizes, and other sources of every Battle Items-pocket item. Keep the item IDs and records intact, and audit existing saves only for harmless unusable leftovers. |
| Trainer and wild content rebalance | In progress | Early-area land and water tables are being rebuilt around the finalized cap curve. Redesigned tables default to fixed morning/day/night content, and radio, swarm, and night-fishing replacements must not introduce species outside the reviewed area list unless explicitly planned otherwise. |
| Fishing encounter rates | Planned | Revisit the bite rates for each rod. Cherrygrove and the other currently redesigned water tables use 25% for the Old Rod, 50% for the Good Rod, and 75% for the Super Rod. Decide whether the new rates should be standardized across areas or configured per area before changing them. |
| Trainer AI changes | Pending | First enable the strongest suitable existing trainer AI, then add a trainer-only fair-information decision layer, switching and item evaluation, doubles coordination, and bounded search. Wild and scripted AI must retain their original routes. |
| Graphics and presentation | Pending | Replace the temporary Bait icons, redesign rival battle/overworld graphics, add challenge messages, and consider title-screen changes after core systems stabilize. |
| Full-game regression pass | Pending | Perform milestone, save/reload, encounter, progression, and hardware/emulator checks after the remaining systems and content are integrated. |

## Level-cap milestones

The authoritative cap table, trainer IDs, save behavior, and exclusions are in
[`LEVEL_CAPS.md`](LEVEL_CAPS.md). Every requested milestone remains an
independent table entry, including Pryce, Clair, and the Kanto leaders whose
current rewards do not change the cap.

## Code budget

Injected-code, packaged-data, save, heap, stack, and VRAM costs are tracked in
[`CODE_BUDGET.md`](CODE_BUDGET.md). Update its link snapshot and affected
feature rows after each user-requested successful build that changes runtime
code or memory use.

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
the overworld. Registered-button use creates that same field task directly, so
both entry paths share the heal and message behavior.

## Revised opening sequence

The consolidated Mum, Elm, assistant, Silver 1, counterpart tutorial,
Cherrygrove guide, Mr. Pokemon Egg, phone call, and Route 30 state design is in
[`OPENING_SEQUENCE_PLAN.md`](OPENING_SEQUENCE_PLAN.md). It preserves the
normal Lyra/Ethan counterpart selection and records every skipped vanilla
state that must be reproduced rather than bypassed.

## Permanent death and wipe recovery

The centralized final-HP deletion policy, transactional reserve selection,
script-driven post-fade notifications, no-reserve defeat-to-title ending,
memory impact, and verification matrix are in
[`PERMANENT_DEATH_PLAN.md`](PERMANENT_DEATH_PLAN.md).

## Laptop PC access

The independent Laptop item, shared physical-PC launcher, safe field-use
lifetime, Mum's initial gift, and implementation steps are in
[`LAPTOP_PLAN.md`](LAPTOP_PLAN.md).

## HM field actions

The machine-ownership model, centralized move mapping, direct-interaction
hook, Party-menu integration, preserved field checks, scope exclusions, and
verification matrix are in
[`HM_FIELD_ACTIONS_PLAN.md`](HM_FIELD_ACTIONS_PLAN.md). The standard
Pokémon-driven field-move presentation is retained.

## Trainer interaction and victory rewards

- `data/trainer_rewards.csv` is the readable source for one optional item and
  quantity per trainer ID. Omitted trainers have no reward. The host-side
  trainer generator rejects unknown or duplicate trainer IDs, mismatched zero
  item/quantity pairs, invalid item IDs, and invalid quantities.
- The generator reads the trainer's authoritative party and existing species
  and item names, then adds a dedicated offer message to the trainer-text
  archive. Teams and reward names are therefore not copied into manually
  maintained dialogue or linked C tables.
- Every undefeated trainer entering the shared normal-trainer talk script shows
  its existing introduction followed by the generated offer and standard
  Yes/No menu. Selecting No closes the interaction without changing trainer,
  reward, phone, or rematch state. Talking again repeats the offer.
- A configured reward is checked for Bag space before the battle. A full Bag
  displays the standard response and leaves the trainer undefeated. Winning
  awards the item before the existing trainer flag is set, so the first battle
  cannot be repeated for another reward during normal play.
- Existing defeated and rematch branches are unchanged and award nothing.
  Automatic fights and battles owned by custom map scripts are intentionally
  outside this first implementation. They can be audited later if their
  individual control flow warrants the same offer.
- The feature adds trainer script and message-archive data but no linked ARM
  code, save fields, heap allocations, graphics, or VRAM use.

## Trainer line of sight

Automatic trainer detection must be stopped before it takes control of the
field, moves a trainer toward the player, or starts the sight-triggered trainer
script. Merely ending that script after detection is not sufficient because it
would still interrupt the player. Talking to an undefeated trainer through the
shared normal-trainer script uses the preview and Yes/No offer described above.
Talking after victory must retain the normal defeated dialogue and rematch
behavior. A trainer whose own data defines a double battle remains a double
battle. Two otherwise independent trainers are not combined merely because
their former sight lines overlap; the player talks to and battles each one
separately.

## In-battle items and Battle Item availability

- `DISABLE_ITEMS_IN_TRAINER_BATTLE` intercepts a player item selection in a
  trainer battle, displays the standard cannot-use message, and returns the
  player to command selection without using the item. The Bag remains
  selectable and browsable; this is accepted for the initial implementation.
- Ordinary wild-battle Bag behavior remains unchanged. Any normally valid
  Ball or other active Bag item can still be used there.
- Safari Zone and Bug-Catching Contest capture controls remain unchanged.
- Held-item effects remain active. This option only changes player commands;
  enemy trainer behavior is unchanged.
- Every item assigned to `POCKET_BATTLE_ITEMS` is removed from normal
  acquisition. This includes the X items, Guard Spec., Dire Hit, Poké Doll,
  Fluffy Tail, and the battle-use flutes in the current item data.
- Existing item IDs and item-data entries remain stable. Marts, hidden items,
  visible pickups, gifts, prizes, scripts, and other acquisition tables must
  be audited, with removed rewards replaced deliberately rather than leaving
  empty interactable locations.

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
