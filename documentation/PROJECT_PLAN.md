# Pokémon Heartless Gold Project Plan

Last updated: 2026-07-31

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
- Prevent the player from using active items in battle. Trainer battles omit
  Bag access; ordinary wild battles retain access to every valid capture Ball
  in the Ball pocket so the capture rules remain playable. Held items are
  unaffected.
- Remove normal acquisition sources for every item in the Battle Items pocket.
  Retain their IDs and item data for archive stability.

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
| Reusable healer | Complete; manually verified | Professor Elm gives new players a reusable Healing Kit after they receive their starter. It fully restores party HP, PP, and major status outside battle, is never consumed, and cannot be used in battle. See `REUSABLE_HEALER_PLAN.md`. |
| Permanent death and wipe recovery | Implemented; build and manual verification pending | Eligible fainted party Pokémon are deleted centrally after battle. Common field scripts report deaths and transactional reserve recovery after the overworld is visible; no-reserve wipes show the ending message and return to title. See `PERMANENT_DEATH_PLAN.md`. |
| Laptop PC access | Planned; independent from permanent death | Add a reusable Laptop Key Item that opens the shared PC menu from safe field contexts. Mum initially gives it during the existing Pokégear sequence. See `LAPTOP_PLAN.md`. |
| Forced-female protagonist | Pending | Skip gender selection, write the female profile value, continue to name selection, and retain the standard Lyra graphics without replacing Ethan's unrelated NPC assets. Existing saves do not need migration. |
| HM field actions | Implemented; build and manual verification pending | Owned machines enable Cut, Surf, Strength, Rock Smash, Waterfall, Whirlpool, Rock Climb, Fly, or Flash without teaching or compatibility requirements while preserving the original field checks and Pokémon presentation. See `HM_FIELD_ACTIONS_PLAN.md`. |
| Evolution modernization | Partially complete; upstream mechanics present | hg-engine already uses the modern friendship threshold of 160, provides Linking Cord routes for the original trade evolutions and held-item trades, and replaces the magnetic-field, moss-rock, and ice-rock evolutions with stones. Player-facing completion still requires normal acquisition sources for the Linking Cord and required held items, plus an audit of unsupported special methods such as Karrablast/Shelmet's paired trade. |
| Capture challenge rules | Implemented; manual verification pending | Ordinary encounters, Safari's shared saved opportunity, duplicate enforcement, the shiny clause, and first-retained-Pokémon Contest handling are implemented. Focused Safari and revised Contest retention checks remain. See `CAPTURE_RULES_PLAN.md`. |
| Bug-Catching Contest availability | Pending after capture rules | Remove the weekday restriction so the Contest can be entered every day. Preserve other entry requirements and the existing daily participation limit unless changed separately. |
| Generation 5-6 species engine/data foundation | Complete; supplied by hg-engine | The repository contains the expanded species IDs, personal data, evolutions, learnsets, experience data, battle and follower graphics, icons, cries, forms, and expanded Pokédex tables. This records the upstream foundation as complete for project planning; it is not a claim that every species and form has received focused in-game verification. |
| Generation 5+ Pokémon content integration | Deferred | Select and place the expanded roster in encounters, trainers, gifts, and other acquisition sources, then balance it against progression. The current encounter and trainer content does not make the compiled species available automatically. |
| Talk-initiated trainer battles | Planned | Disable automatic sight detection before it starts trainer movement or scripts, while preserving the existing talk interaction, defeated dialogue, rematches, and paired/double-trainer behavior. |
| Trainer victory rewards | Planned | Give each configured enemy trainer ID its item and quantity once, after the first victory only. Keep the authoritative trainer-to-reward mapping in generated ROM data, with rematches configurable as no reward, and persist claimed rewards independently from resettable trainer flags. Audit ordinary, sight-capable, double, rival, Gym, Elite Four, rematch, and other scripted trainer paths. |
| In-battle player item restriction | Planned | Omit the Bag in trainer battles and permit every valid capture Ball from the Ball pocket in ordinary wild battles. Preserve Safari and Bug-Catching Contest capture commands, held-item effects, and enemy trainer item behavior. |
| Battle Item acquisition removal | Planned | Remove or replace marts, visible and hidden pickups, gifts, prizes, and other sources of every Battle Items-pocket item. Keep the item IDs and records intact, and audit existing saves only for harmless unusable leftovers. |
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

- The reward source will be a readable data file such as
  `data/trainer_rewards.csv`, with symbolic trainer ID, item ID, and quantity
  columns. `ITEM_NONE` with quantity zero explicitly configures a trainer ID
  to give no reward. A build-time validator will reject unknown IDs, duplicate
  trainer rows, and invalid item/quantity combinations.
- The build will convert that file into a fixed-format member of
  `ARC_CODE_ADDONS`. Runtime code will read the record for a trainer ID
  directly; the full mapping will not occupy the fixed injected-code region.
- Each distinct nonzero enemy trainer ID in a won battle is considered once,
  and only if its configured reward has never been claimed. A two-trainer
  battle can therefore award two configured first-victory rewards, while a
  single trainer using two battler slots cannot award twice.
- Rematch trainer IDs will be configured as no reward. Repeating a battle with
  an already rewarded trainer ID also gives nothing, even if the underlying
  trainer-defeated flag is later cleared or reused.
- A save-backed claimed-reward bit indexed by trainer ID will enforce the
  one-time rule independently from trainer flags. At the current 740 trainer
  IDs this requires approximately 93 bytes--rounded to about 96 bytes when
  stored as aligned words--in expanded save data. The bit is set only after
  the item has actually been added to the Bag.
- Reward presentation must run from a verified post-victory field-script
  context and use the standard item-obtained and Bag-full flows. The detailed
  implementation plan must decide how an unclaimed Bag-full reward is retained
  rather than silently lost.
- The trainer-script audit must cover the shared ordinary and sight-triggered
  paths as well as story and map scripts that launch trainer battles directly.

## Trainer line of sight

Automatic trainer detection must be stopped before it takes control of the
field, moves a trainer toward the player, or starts the sight-triggered trainer
script. Merely ending that script after detection is not sufficient because it
would still interrupt the player. Talking to an undefeated trainer must retain
the normal pre-battle dialogue and battle flow; talking after victory must
retain the normal defeated dialogue and rematch behavior.

## In-battle items and Battle Item availability

- Trainer battles do not expose a usable Bag command to the player.
- Ordinary wild battles expose the complete Ball pocket required for capture.
  Any Ball that is valid for the current encounter may be used; this is not
  limited to the item named Poké Ball. Healing items, status items, escape
  items, Battle Items, and other active Bag items cannot be selected or used.
- Safari Zone and Bug-Catching Contest capture controls remain available.
- Held-item effects remain active. Enemy trainer item use is not changed by
  this player-facing restriction.
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
