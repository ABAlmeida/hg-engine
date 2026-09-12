# Trainer AI Semantics Coverage

This is the nonblocking evaluator backlog for expert singles AI. It records
what the resident evaluator understands without making ordinary builds depend
on complete strategic coverage.

## Release disposition

The resident expert-singles AI is complete and accepted for the current
release. Unchecked entries below are optional hardening opportunities, not open
release requirements. Gameplay evaluation found that short manipulation
sequences could influence the AI before its prediction memory adapted and
countered them. Reopen this backlog only for a repeatable manipulation that
remains exploitable after adaptation, a concrete incorrect decision pattern,
or newly reachable content whose semantics materially affect decisions.

The runtime source of truth remains `data/trainer_ai_semantics.json`, the move
table and the handwritten handlers in `src/battle/trainer_ai.c`. This report is
human-readable tracking data and is never linked into the ROM.

## Status definitions

| Status | Meaning |
|---|---|
| Exact | The relevant legality, timing, probability and state transition are modelled for the supported singles scope. |
| Generic | Shared mechanics correctly describe the move; no move-specific handler is needed. |
| Conservative | The move is legal and receives a documented low action-local value pending or instead of bespoke strategy. |
| Ignored | The special effect does not materially change tactical selection in the supported scope, and ordinary components remain evaluated. |
| Unimplemented | The engine marks the move unused; expert AI excludes it. |
| Needs work | A known reachable mechanic is evaluated incorrectly or incompletely. |
| Investigate | Reachability or required semantics have not yet been classified. |

`Needs work`, `Investigate` and deliberate `Conservative` entries do not block
`make`, `make code`, CI or ROM packaging. An incomplete action must never set a
global fallback, randomize understood actions or remove other legal choices.

## Current compact generated data

| Class | Status | Representation | Notes |
|---|---|---:|---|
| Primary/secondary move effects | Generic | 30 packed `u16` effect/class entries (60 bytes) | One compact lookup selects shared status, recovery, field, combined-stage, protection or flinch handling; numeric move data is not duplicated. |
| Move-only classes | Generic | 11 packed `u16` entries | Screens, hazards, Tailwind, Substitute and random-action moves whose effect ID alone is insufficient. |
| Ability/type interactions | Generic | 12 packed `u16` entries | Immunities, absorption, active Flash Fire damage, Thick Fat and Dry Skin use minimum evaluator-facing behavior. Other absorption stat gains are deliberately omitted from the one-turn screen. |
| Small priority/status/item rules | Generic | Direct shared handlers | The compact direct comparisons are smaller than separate tables at the current low cardinality; move/species-specific scoring is not introduced. |
| Consecutive move transitions | Generic | 3 packed `u16` entries | Rollout and Ice Ball share forced-repeat scaling; Fury Cutter shares selectable repeat scaling. Active progress is represented generically rather than with per-move request fields. |

The effect classes are sparse because only a small fraction of the
`0..405` effect range needs strategic dispatch today. A mostly-zero 406-byte
table would consume resident headroom without changing decisions. The packed
primary entry uses nine bits for the effect ID and seven for the shared class;
the generator fails if either bound is exceeded.

The listed generated arrays contain 112 logical payload bytes before linker
alignment. That is a source-level count, not a linked-size claim; the next
explicitly authorized build must measure the actual `.rodata` and total
overlay-130 effect against the 500-byte reserve.

## Implemented shared handling

| Family | Status | Current scope |
|---|---|---|
| Ordinary physical/special damage | Generic | Level, power, exact offensive/defensive stats, stages, accuracy, STAB, type effectiveness, compact ability/type rules and active Flash Fire state. Primary HP utility is capped at remaining HP, while a compact uncapped damage-pressure value breaks exact final-score ties. Status moves return zero damage; validity is represented by action utility, never a negative damage sentinel. |
| Basic fixed damage | Generic | Sonic Boom, Dragon Rage, Seismic Toss, Night Shade, Psywave, Super Fang and Final Gambit. |
| OHKO moves | Conservative | Bounded expected damage; exact legality and hit formula still need work. |
| Move order | Generic | Priority, supported priority abilities, exact staged Speed, Trick Room and true ties. |
| Flinch secondaries | Generic | Hit/effect probability and exact move-order applicability. |
| Basic self stages | Generic | Attack, Defense, Special Attack, Special Defense and Speed changes with cap checks, exact current Speed-order transitions and analytic payoff. Combined effects retain useful uncapped components rather than failing when one component is capped or tactically neutral. |
| Basic target stages | Generic | Immediate effect plus discounted future persistence when the target can switch. |
| Poison/bad poison/burn/paralysis/sleep | Generic | Existing status, common type/ability immunity, accuracy and current tactical value. Sleep includes miss exposure and move order. |
| Leech Seed | Generic | Immediate drain/healing, retained value and switch-forcing tempo. |
| Recovery/Rest/Roost | Generic | Actual missing HP, status and incoming threat. |
| Screens and Tailwind | Generic | Existing-side checks and current-matchup damage or move-order benefit. Reserve-wide duration planning is deferred. |
| Entry hazards | Conservative | Layer/existing-state legality and low setup value; full remaining-team payoff needs work. |
| Weather | Conservative | Low action-local value; team composition, duration and opposing weather benefit need work. |
| Trick Room | Conservative | Current speed-order check and low immediate value; full team-duration evaluation needs work. |
| Protect/Endure | Generic | The fork's shared `1, 3, 9, 27, 81, 243, 729` success denominators are applied to expected incoming damage and lethal asset risk. Endure is admitted only against expected lethal damage. Contact punishments, residual-turn value and complete side-guard semantics remain conservative. |
| Curse | Generic | Ghost Curse rejects an already cursed or substituted target and weighs HP sacrifice against switch-discounted residual value. Non-Ghost Curse combines every useful uncapped Attack/Defense/Speed transition and charges the setup turn once. |
| Party status cure | Generic | Heal Bell/Aromatherapy are invalid when no trainer party member has a curable major status; otherwise value scales with affected members. |
| Substitute | Conservative | Immediate HP checks; full branch semantics need work. |
| Unused moves | Unimplemented | Filtered through `FLAG_UNUSED_MOVE`. |
| Unclassified legal effect | Conservative | `AI_CFG_UNKNOWN_ACTION_BASE_VALUE` applies only to that action. |

## Prioritized future-hardening backlog

### P0 — decision integrity

- [x] Remove global unknown-action randomization.
- [x] Remove arbitrary near-best random action selection.
- [x] Retain uncapped damage pressure and use it only as a deterministic
      tie-break between actions with exactly equal primary scores.
- [x] Treat known unequal Speed as deterministic.
- [x] Remove mandatory post-entry action and recent-switch penalties.
- [x] Prevent a Ghost-type user from selecting Curse when the concrete target
      already has `STATUS2_CURSE` or a Substitute. The generated
      `MOVE_EFFECT_CURSE` class also values the HP sacrifice, switch-discounted
      residual payoff and every useful non-Ghost Attack/Defense/Speed change.
- [ ] Remove actions dominated across matching concrete branches. The obsolete
      stay/switch/absolute-worst shortcut has been removed; branch-vector
      dominance remains follow-up work.
- [x] Replace the coarse aggregate player-switch estimate with probabilities
      for every concrete move and switch destination.
- [ ] Construct the final four response-distinct trainer actions from the
      complete cheap response scan; the current resident scorer ranks its
      admitted move/switch set directly.
- [ ] Calculate a mixed strategy only from non-dominated pairwise payoffs and
      counter-reading evidence.

### P1 — high-impact battle mechanics

- [ ] Complete ability/item/weather/screen/burn/critical modifiers in damage.
- [ ] Complete variable-power, multi-hit, recoil, drain, charge, recharge,
      pivot and phasing damage/effect families. Rollout, Ice Ball and Fury
      Cutter now have generic locking/repeat progression, but the remaining
      families are still open.
- [ ] Model exact accuracy/evasion and move-specific failure conditions.
- [ ] Complete stochastic move-order items and abilities without introducing
      generic Speed uncertainty.
- [ ] Complete status immunity/prevention interactions and field protections.
- [ ] Model secondary effects other than flinch as conditional state
      transitions rather than flat bonuses.

### P2 — setup, support and switching

- [ ] Retain exact remaining duration and removal branches for Light Screen,
      Reflect and Tailwind.
- [ ] Value hazards over every living reserve using HP, typing, grounded state,
      removal and expected future entry.
- [x] Evaluate repeated Attack/Special Attack setup while each additional use
      improves the bounded survival/KO plan.
- [ ] Extend repeated setup transitions to defensive, Speed and combined-stat
      plans with the same survival accounting.
- [ ] Evaluate support sequences such as screen -> Tailwind -> switch with a
      receding horizon and opponent responses on every turn.
- [ ] Replace binary target-drop persistence discount with concrete future
      switch cost, hazards, depleted/fainted counters, trapping and last-mon
      state.
- [ ] Retain immediate drain/control and forced-switch tempo for all relevant
      target-bound effects.
- [x] Screen all reserves on the common pressure/threat/HP position score and
      admit at most two final switch actions. Ordinary entries must survive and
      improve the resulting position unless the active Pokémon faces an
      immediate KO. Setup/support utility is evaluated after entry rather than
      duplicated in the cheap reserve scan.
- [x] Apply the same discounted continuation horizon to move/move,
      move/switch, switch/move and switch/switch branches.
- [x] Use the shared attack/support/threat position score for post-KO
      replacement rather than HP/asset value alone.

### P3 — catalog completion

- [ ] Promote currently conservative specialized effects only when their
      reachable trainer sets justify resident code. This includes Encore,
      weather, Defense Curl, Minimize, Charge, confusion,
      critical-rate setup, Haze, phasing, Disable, Torment, Spite, Lock-On,
      Safeguard, Belly Drum, Nightmare, Psycho Shift, Ingrain,
      Magnet Rise and Perish Song. Until then each receives the ordinary low
      action-local conservative score and cannot disturb other actions.
- [ ] Inventory every implemented move reachable by expert trainers.
- [ ] Inventory every implemented player-obtainable move, including forms and
      shared learnsets.
- [ ] Classify every effect as Exact, Generic, Conservative, Ignored,
      Unimplemented, Needs work or Investigate.
- [ ] Add narrow move-ID overrides where an effect ID does not fully describe
      the mechanic.

## Maintenance rules

When evaluator work or reachable content changes:

1. update the relevant family or backlog entry here;
2. update `data/trainer_ai_semantics.json` when runtime class data changes;
3. keep unsupported behavior action-local;
4. do not add release strings or diagnostic names to resident data; and
5. measure generated data and overlay headroom only during a separately
   authorized build.
