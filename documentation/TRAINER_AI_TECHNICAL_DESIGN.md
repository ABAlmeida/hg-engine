# Heartless Gold Expert Trainer AI Technical Design

An exact-information, action-blind, prediction-aware singles trainer AI for
Pokémon HeartGold hg-engine.

| Field | Value |
|---|---|
| Status | Complete and accepted for the current release (v0.6) |
| Scope | Expert singles trainer battles only |
| Production profile | `F_TRAINER_EXPERT_AI` |
| Primary goals | Strong tactics, smart switching, multi-turn planning, player-action prediction, bounded runtime, measurable size |
| Explicit non-goals | Doubles AI, full battle-script simulation, deep game-tree search, runtime machine learning, reading the player's selected command |
| Source baseline | Accepted resident implementation with original HGSS fallback; future work is reopened only for demonstrated defects or repeatable exploits |

> **Document status:** The resident expert-singles AI is complete and accepted
> for the current release. This document retains both the implemented contract
> and intentionally deferred future refinements; a deferred item is not open
> project work unless gameplay demonstrates a defect or repeatable exploit.
> Section 3 inventories the removed committed legacy AI as regression and
> integration evidence. The exact historical rollback is specified in
> `TRAINER_AI_LEGACY_ROLLBACK_CHANGELIST.md`.

# 0. Restart and revision policy

Both the discarded v0.4 overlay/lookahead attempt and the earlier committed
fair-information AI have been removed from the current source worktree. The
legacy removal was a surgical source rollback, not a whole-commit inverse,
because the introducing commit also contained a general move-data fix and
trainer-data cleanup.

Preserve only:

- tactical requirements and integration lessons recorded from the committed
  legacy AI in Section 3, without retaining its runtime implementation;
- general engine fixes that are not owned by the AI feature;
- integration seams that were statically verified even if the committed
  legacy AI did not yet use them; and
- lessons learned from the discarded attempt.

Four states must remain distinct in implementation notes and size reports:

1. **Removed committed legacy AI:** the fair-information implementation
   introduced by `ad0d002a1f4dc2eb20782391bdfb39ec58ebee17`.
2. **Current clean rollback baseline:** original HGSS AI selected through the
   preserved trainer flags, plus unrelated Heartless Gold fixes and content.
3. **Discarded attempt:** the v0.4 overlay/lookahead scaffold that produced the
   battle-entry and per-turn performance investigation.
4. **v0.6 target:** the design in this document.

No generated file, stale object or ROM output is evidence that a reverted
source tree implements v0.6. Phase 0 establishes a fresh source and size
baseline after both AI rollbacks.

## 0.1 Major replacement-design changes

- Replace the old fair-information belief model with an
  **exact-information, action-blind** contract. The AI may know the player's
  complete party, final calculated stats, moves, PP, ability and held item,
  but it may never inspect the command, move, switch destination or target the
  player selected for the current turn.
- Remove move-possession inference, natural-learnset loading, ambiguous ability
  candidates, hidden-item beliefs and IV/EV/nature estimation. Copy final
  calculated stats instead of retaining raw IVs, EVs or nature.
- Keep uncertainty only where it is real and useful: which legal action the
  player will choose and how likely each action is.
- Implement the complete expert AI in resident overlay 130 first. Exact
  information, compact shared mechanics handlers and generated coverage data
  must be used to preserve the 500-byte reserve.
- A transient overlay is an unapproved contingency, not an implementation
  phase or automatic fallback. If a measured resident implementation cannot
  fit, stop and obtain explicit user approval before reserving an overlay ID,
  adding a linker image or loader, or moving any evaluator code transient.
- Do not add an “obvious decision” fast path in the first v0.6 implementation.
  One cached non-forced expert decision runs the resident planner once.
- Separately measure snapshot construction, tactical primitives, switch
  screening and search. The discarded attempt did not do this and therefore
  cannot establish which component caused its pause.
- Add analytic multi-turn plan evaluation for setup, stat changes, recovery and
  support. A six-turn payoff must not require a six-ply search tree.
- Screen every legal trainer reserve on one compact resulting-position score,
  then admit at most the best two distinct reserves to the final scored action
  set. The score combines entry survival, best pressure, incoming threat and
  remaining HP, so pivots and attackers compete without resident role arrays.
  Setup/support is evaluated normally once that Pokémon is active. Three
  reserve branches crowded out moves, increased runtime and amplified switch
  over-selection.
- Make coverage and maximum regret enforceable selection constraints. A
  damaging move into an immunity is not a safe action merely because it is
  strong against one predicted switch.
- Track semantic coverage for every implemented move reachable by either an
  expert trainer or the player in
  `documentation/TRAINER_AI_SEMANTICS_COVERAGE.md`. Missing or deliberately
  conservative mappings never block an ordinary build and never randomize
  understood actions; the report is the prioritized evaluator backlog.
- Implement prediction commitment/resolution, recent-battle adaptation and the
  32-case abstract persistent memory described here. They did not exist in
  either the committed legacy AI or the discarded implementation.
- Treat Bag use as disabled battle functionality for both sides. The expert AI
  neither generates trainer ITEM actions nor predicts player ITEM actions;
  configured trainer-item data may remain inert in content.
- Evaluate every specific legal player move and switch in the immediate
  prediction/coverage pass. Four player branches limit only expensive future
  simulation, not action awareness.
- Generate compact move-semantics data at build time and include it through one
  resident implementation unit. JSON parsing and descriptive names never
  enter the ROM. Coverage auditing is a separate nonblocking maintenance task.
- Put all preference, breadth and risk tuning in `include/ai_config.h`. Battle
  mechanics remain fixed and cannot be changed by tuning parameters.

## 0.2 Accepted resident implementation

The current source implements the guarded expert-singles hooks, exact party
snapshot, resident battle-owned state, command/move cache, original-HGSS
fallback, post-KO selector, compact flinch class, tactical evaluator,
voluntary switching and prediction-case storage. The latest evaluator revision
also:

- uses deterministic priority/Speed order except for true ties;
- permits a switch immediately after any entry and contains no switch-loop
  penalty or cooldown;
- predicts a move from the current matchup before applying its damage to a
  proposed switch-in;
- uses one continuation horizon for move/move, move/switch, switch/move and
  switch/switch pairs, rather than granting future value only to switches;
- admits at most two highest-position voluntary switches, rejects an entry that
  is expected to faint immediately and requires an ordinary switch to improve
  the resulting matchup by the configured margin unless the active Pokémon is
  already in immediate KO danger;
- selects post-KO replacements with the same attack/support/threat position
  score used by voluntary-switch screening rather than HP/asset value alone;
- resolves status, flinch, type interaction and consecutive-move classes
  through generated mechanics tables, with direct shared handlers for the few
  priority/status/item rules where a table would cost more;
- values Sleep accuracy/miss exposure and switchable target effects
  independently;
- scores every admitted action against the same concrete player-action
  distribution and applies probability-weighted downside risk;
- retains each move's uncapped, accuracy-adjusted damage pressure separately
  from capped HP-loss utility and uses it only to break exact primary-score
  ties;
- selects the best current prediction/risk policy without arbitrary near-best
  RNG; and
- confines incomplete semantics to the affected action.

The current implementation is the accepted v0.6 release scope. Probability
calibration, fuller move mechanics, duration-aware support sequences and
calculated mixed strategies remain documented in
`TRAINER_AI_SEMANTICS_COVERAGE.md` as optional future hardening, not unfinished
release work. Gameplay evaluation showed that some short manipulation sequences
could initially influence the AI, after which its prediction memory recognized
and countered the pattern. Reopen tactical work only if a manipulation remains
repeatably exploitable after adaptation or another concrete regression is
demonstrated.

# 1. Executive summary

The expert AI is a bounded decision system for singles trainer battles. It
knows both parties' complete battle-relevant loadouts but remains blind to the
player's current command. It compares moves and switches on a compatible
utility scale, predicts several plausible player actions, selects robust lines
when available and records an explicit prediction commitment only when it
chooses a specialized read.

The design has five layers:

1. A resident integration layer validates the battle format/profile, snapshots
   exact state, generates legal actions, caches decisions and provides a legal
   tactical fallback.
2. A compact tactical model calculates damage, KO timing, speed, survival,
   hazards, status and other immediate effects from exact sanitized values.
3. A plan evaluator compares multi-turn attack, setup, recovery, support and
   switch plans without expanding a deep game tree.
4. A player-action model assigns probabilities to every exact legal move and
   switch destination. Every branch participates in immediate coverage; only
   the most relevant four are candidates for expensive future simulation.
5. A resident bounded planner performs pairwise coverage, future evaluation,
   prediction commitment generation and final ranking.

Bag commands are disabled for both the player and trainer in the supported
battles. The expert AI never generates, predicts or returns ITEM. Configured
trainer Bag-item data may remain but is inert. Doubles and unsupported formats
delegate to the original HGSS AI.

# 2. Goals, non-goals and invariants

## 2.1 Goals

- Take guaranteed KOs and respect immunities, accuracy, priority and speed.
- Compare attacking, recovery, setup, support and switching in one decision.
- Understand multi-turn payoff such as Leer plus four Tackles versus six
  immediate Tackles.
- Prefer persistent self boosts over removable target drops when the position
  warrants it, without using a universal hard-coded preference.
- Select useful defensive pivots, attackers and support/setup reserves.
- Predict player actions without reading the selected command.
- Remember genuine read outcomes and adapt probability/mixup behavior without
  an automatic failed-read repeat penalty.
- Preserve at least 500 linked bytes in overlay 130.
- Keep runtime bounded and ordinary decision latency below two video frames.
- Measure strength, size and runtime at every implementation phase.

## 2.2 Non-goals

- Doubles, multi, tag or other multi-battler strategy.
- Full battle-script execution in the evaluator.
- Deep minimax over every future player response.
- Runtime learning of move mechanics or trainer data.
- Reading a player's current selected move, switch, target or command.
- Retaining trainer, Pokémon, move, item, ability, slot or matchup identities
  in persistent prediction memory.
- A large release diagnostic or explanation framework.
- Modelling player or trainer Bag actions; both are disabled and outside the
  action space.

## 2.3 Non-negotiable invariants

| Invariant | Requirement |
|---|---|
| Command blindness | The AI may read exact loadout/state but never current player command, selected move slot, switch destination or target. |
| No indirect command leak | No helper used before selection may infer the current player command from battle-controller fields. |
| Exact snapshot boundary | Strategic evaluation consumes the sanctioned exact snapshot and derived public mechanics, not selected-command fields or unrestricted live battle pointers. |
| Legal action only | Every returned move/switch index is validated resident-side before use. |
| Unsupported-format fallback | Doubles and unsupported formats delegate before singles-only state is accessed. |
| One production profile | Only `F_TRAINER_EXPERT_AI` selects the advanced path. |
| No Bag action | Bag use is disabled for both sides; expert singles never generates, predicts or selects ITEM. |
| Cached decision | Command and move hooks consume the same decision; decision work runs once. |
| Coverage is enforceable | Coverage and regret thresholds affect eligibility, not merely a small score bonus. |
| Tracked reachable coverage | Implemented move semantics are classified in the nonblocking Markdown backlog; any fallback remains action-local. |
| Prediction honesty | Robust actions create no read record; unavoidable losses create no false player-win record. |
| Scoped memory | A small recent-battle window clears between trainers; 32 abstract player-behaviour cases persist without trainer, species, move, item or slot identity. |
| Resident headroom | Overlay 130 retains at least 500 linked bytes. |
| Bounded state | Battle-owned AI state/workspace is at most 768 bytes; overlay BSS remains minimal and separately measured; worst nested AI stack is below 1 KiB. |
| Transient approval | No transient overlay ID, linker image, loader or evaluator placement may be created without explicit user approval after measured resident overflow. |
| Purposeful variation | Evaluation consumes no production battle RNG; selection samples only a calculated non-dominated mixed strategy when prediction warfare justifies one. |

# 3. Removed legacy AI inventory and rollback boundary

This section documents the removed fair-information AI from commit
`ad0d002a1f4dc2eb20782391bdfb39ec58ebee17`: its evaluator, interface, hooks
and battle-start reset. Its covered mechanics and verified seams remain
evidence for v0.6 requirements, not code in the current baseline.

## 3.1 Verified integration evidence to retain in the design

- The legacy runtime uses `IMPLEMENT_FAIR_TRAINER_AI` and
  `TrainerAI_UsesStrategicLayer()`. Both are removed with that runtime.
- The overlay-12 command and move call sites have verified addresses:

  ```text
  0012 FairTrainerAI_PickCommand 0225E10E bl
  0012 FairTrainerAI_PickMove    0225E43E bl
  ```

- The legacy hook lines are removed for the clean rollback baseline, then v0.6
  reintroduces guarded wrappers at the verified addresses with instruction
  assertions.
- The original HGSS picker addresses and fallback behavior are known. Linker
  exports are removed if unused during rollback and re-added only when v0.6
  needs them.
- `F_TRAINER_EXPERT_AI` is preserved. It predates the legacy implementation,
  expands to the three original HGSS expert modules and remains the production
  content profile.
- The v0.6 command and move selections must share a cache keyed by turn and
  active party slot.
  This prevents two independent decisions for one turn and protects against a
  replacement inheriting a fainted Pokémon's decision.
- The legacy ability/held-item observation calls are removed. Exact-information
  v0.6 refreshes its value snapshot through one central sanitizer instead of
  preserving the old observation subsystem.
- The general `GetMoveData(..., MOVE_DATA_PSS_SPLIT)` correction is an engine
  fix, not AI-owned behavior. Do not revert it with the AI feature.

## 3.2 Legacy tactical behavior to preserve as requirements or subsume

The old AI already provided:

- one action list containing usable moves and legal voluntary switches;
- PP, disabled-move and Struggle legality checks;
- approximate damage, STAB, type effectiveness, common ability immunities,
  accuracy and fixed-damage handling;
- priority and approximate speed-order reasoning;
- common status, recovery, setup, screens, hazards, weather, speed control,
  protection, substitution and disruption scoring;
- legal reserve checks and trapping checks;
- reserve scoring based on expected incoming damage and the reserve's best
  contextual next action, including support/setup rather than damage only;
- a recent-switch penalty;
- protection against zero-progress chains where a newly entered trainer
  Pokémon immediately switches again before taking an action;
- a coarse player-switch model and safe near-best randomization; and
- quantized player HP rather than exact player HP, which v0.6 intentionally
  replaces with exact sanitized state.

These algorithms are removed with the legacy runtime. They are not
automatically correct, but their covered mechanics form a regression inventory
for the replacement. v0.6 explicitly rejects the listed recent-switch penalty,
mandatory action after entry and safe-near-best randomization; they are retained
above only as historical evidence of the behavior that produced the observed
tactical failures.

## 3.3 Legacy behavior to replace

The old AI also had important limitations:

- It inferred up to eight player moves from level-up learnsets and observations
  instead of reading the exact four-move loadout.
- It estimated player stats from species/level and repeatedly queried personal
  data during evaluation.
- It modelled player switching as a crude probability against one reserve,
  chosen primarily by HP rather than switch suitability.
- It evaluated every trainer reserve deeply, often against both the active
  target and one predicted reserve.
- An evaluator-unknown status action set a global `unknown` flag; the entire
  decision then became uniformly random across all legal actions.
- Setup/drop scoring valued mostly the next damage delta and one incoming hit;
  it did not compare complete turns-to-KO plans.
- It had no prediction commitment, read resolution, trainer-local case ring or
  cross-battle calibration.
- It had no dedicated post-KO replacement override.
- The production wrappers checked the expert profile but did not establish the
  v0.6 singles/unsupported-format gate before accessing the custom evaluator.
- Its switch/move scenario model was not a four-branch coverage search.

## 3.4 Verified seam not present in the legacy behavior

Static reconstruction identified the original replacement-fallback call at
overlay-12 address `0x0225F8E8`. The original selector is
`0x02258800 | 1`, receives `(BattleSystem *, u8 battlerId)` and returns party
slot `0..5`, with 6 as the no-selection sentinel.

The call site can be reached during initial party setup as well as after a KO.
A replacement wrapper must therefore delegate unless all of these are true:

- expert singles is active;
- the battler previously completed a real command decision;
- turn zero has ended;
- the battler has an established species; and
- the established battler's HP is exactly zero.

This hook was added by the discarded attempt and is not part of the committed
legacy AI. Reintroduce it deliberately in Phase 1 with the existing BL
assertion.

## 3.5 Applied legacy rollback boundary

The applied authoritative path-by-path checklist is
`documentation/TRAINER_AI_LEGACY_ROLLBACK_CHANGELIST.md`. Its required boundary
is summarized here:

**Removed:**

- `src/battle/trainer_ai.c` and `include/trainer_ai.h`;
- `IMPLEMENT_FAIR_TRAINER_AI`;
- the two `FairTrainerAI_*` hook entries;
- now-unused original-picker linker exports;
- the legacy battle-start reset; and
- the legacy ability and held-item observation calls/includes.

**Preserve:**

- `GetMoveData(..., MOVE_DATA_PSS_SPLIT)` returning `bm->split`;
- `F_TRAINER_EXPERT_AI` and all intentional trainer assignments, including
  additive flags;
- original HGSS AI behavior for the clean transitional baseline and permanent
  unsupported-format fallback; and
- every unrelated battle, trainer, item and progression change.

Do not run `git revert ad0d002a1...` as the implementation. That commit mixes
the removable runtime with the preserved move-split correction and trainer
profile normalization, while later trainer-balance commits depend on the
profile assignments.

Configured trainer Bag-item data may remain, but Bag use is disabled for both
sides. The v0.6 expert-singles action generators do not create trainer ITEM or
player ITEM branches, and no prediction, evaluator or output path handles
them.

# 4. Capacity and memory budgets

## 4.1 Overlay 130

```text
Start:    0x023C4000
Length:   0x00014000
End:      0x023D8000
Capacity: 81,920 bytes
```

`.text`, `.rodata`, COMMON, `.data` and `.bss` all consume this fixed region.
The final `build/output_battle.bin` length, or the highest end of every
allocated linked section, is the capacity measurement. `__end__` alone is not
valid because orphan allocated sections may follow it.

Recorded evidence and the current authorized implementation measurement:

| Source state | Overlay 130 used | Free | Notes |
|---|---:|---:|---|
| Committed legacy-AI measurement | 77,772 B | 4,148 B | Recorded before the discarded attempt; not the clean rollback baseline |
| Estimated committed legacy AI feature | 8,172 B | — | Prior controlled delta; historical only |
| Estimated clean rollback baseline | ~69,600 B | — | Consistent with the dated 69,496-byte disabled build; replace with a fresh post-rollback build |
| Discarded attempt artifact | 80,120 B | 1,800 B | Latest generated artifact inspected during the performance investigation |
| Last authorized pre-amendment resident build | 81,364 B | 556 B | Historical `make quick-rom -j8` result; `trainer_ai.o` contained 10,804 B text, 30 B generated read-only tables and 4 B BSS. The current source is intentionally unbuilt and must not be assumed to have this size. |
| User-run intermediate semantics/switch build | 81,936 B | -16 B | Link failed by 16 B. Its `trainer_ai.o` contained 11,288 B text, 116 B read-only data and 4 B BSS. This measurement triggered removal of duplicated party-asset scans, role arrays and unused tiny tables; the resulting source has not yet been built. |

The discarded attempt also produced a 384-byte AI BSS object and a 676-byte
transient overlay. These numbers describe an incomplete scaffold and are not
v0.6 targets.

The hard resident gate is:

```text
output_battle.bin <= 81,920 - 500 = 81,420 bytes
```

The 500 bytes are linker headroom for resident fixes and integration—not
battle heap. The estimated clean baseline permits approximately 11.8 KiB of
resident replacement AI while retaining that reserve; the value remains an
estimate until measured by an authorized build. Stop before the hard gate
rather than consuming the reserve.

The battle linker region is deliberately capped at `0x13E0C` (81,420 bytes),
500 bytes below the physical `0x14000` region, so the reserve is enforced by
the linker rather than only by a post-link report. The budget script retains a
second check on the produced binary.

## 4.2 Unapproved transient contingency

```text
Start:    0x023C0400
Length:   0x00003C00
End:      0x023C4000
Capacity: 15,360 bytes
```

This existing individual-overlay region is documented only so a future
capacity discussion has an accurate upper bound. v0.6 does not reserve an ID,
link an image, install a loader or place code here. If the complete measured
resident implementation cannot fit while preserving 500 bytes, implementation
must stop and present the resident map, largest symbols and reduction options.
A transient design may begin only after explicit user approval. Approval to
implement the AI or build the ROM does not imply approval to create it.

## 4.3 Other hard budgets

| Memory class | Gate |
|---|---:|
| Battle-owned AI state/workspace | <=768 B on the owning battle heap; lifetime is one battle |
| Overlay-130 AI BSS | No large matrices or duplicate party snapshots; measure separately at every phase |
| Worst nested AI stack | <1 KiB |
| Persistent prediction memory | 32 abstract cases at 6–8 B each (192–256 B) plus a small versioned header; hard target <=272 logical bytes and measure aligned save growth |
| Release diagnostics | No strings or retained score-component arrays |

Initial battle-heap targets are approximately 72–108 bytes for at most nine
concrete player branches, 64–96 bytes for four streamed action summaries and
at most 64 bytes for eight compact recent cases. Damage/plan scratch uses the
remaining bounded workspace and is reused between stages; do not retain a 9x9
or 4x9 matrix. These are design estimates until compile-time `sizeof` reports
and battle-heap headroom are measured. Generated semantic tables consume
overlay `.rodata`, while persistent cases consume save space; neither is part
of the 768-byte battle-owned workspace.

# 5. Architecture and ownership

## 5.1 Resident overlay 130

Resident code owns the complete production implementation:

- profile and singles-format gating;
- battle lifecycle and state clearing;
- exact loadout/state sanitization;
- legal move/switch enumeration and compact move/state descriptor generation;
- decision cache and final engine output;
- a compact, competent tactical fallback;
- player action probabilities and switch-destination ranking;
- trainer switch shortlisting and dynamic role comparison;
- tactical mechanics, secondary-effect outcomes and analytic plans;
- pairwise action/branch outcomes and bounded future-state evaluation;
- coverage, regret, robust eligibility and final action ranking;
- resolving prior prediction commitments after the actual action is known; and
- loading and updating the versioned persistent prediction-case store.

Battle-local cache, commitment, recent-case and bounded scratch ownership lives
in an `AIExpertBattleState` appended to the end of `BattleStruct` after Phase 1
verifies every allocation/clear site and any assembly size assumption. Appending
preserves existing member offsets. Its allocation and clearing follow the
existing battle lifetime; it is not save data and no pointer to it survives
battle teardown. Do not place a large permanent planner matrix in overlay BSS.

## 5.2 Placement rule

Keep the implementation resident and centralize shared mechanics rather than
duplicating per-move code. If measurement proves that the complete release
implementation cannot fit under the overlay-130 hard gate, stop. Do not create
a transient overlay as an optimization experiment or quiet response to linker
pressure. A separately approved transient amendment must define ownership,
safety, build enforcement, runtime cost and fallback before code is added.

The discarded attempt split work across resident and a 676-byte transient image
without first identifying the real performance cost. v0.6 avoids that boundary
and measures one resident pipeline before considering another placement.

## 5.3 Runtime flow

```text
battle state
    -> resident exact-state sanitizer
    -> legal AI/player actions plus compact Pokémon/move descriptors
    -> player distribution and trainer switch screening
    -> tactical caches, secondary outcomes and analytic plans
    -> pairwise coverage and bounded ranking
    -> dominated-action removal and prediction/coverage policy
    -> optional calculated mixed-strategy sample
    -> cached command/move output
    -> later commitment resolution
```

## 5.4 Configuration ownership

`include/config.h` owns only the feature gate that enables the expert-AI
integration. `include/ai_config.h` is the single source of truth for build-time
planner tuning and is included only by AI implementation modules. It contains
compile-time constants rather than a runtime profile object so normal tuning
does not consume battle heap or persistent memory and the compiler may fold
the values into comparisons.

`ai_config.h` owns:

- immediate and future search breadth;
- plan horizon and future discount;
- utility scale and preference weights;
- coverage, acceptable-outcome and regret thresholds;
- comfortable, even, losing and desperate risk tolerances;
- specialized-read admission and payoff thresholds;
- probability smoothing, calibration and safe-mixup breadth;
- switch screening/shortlisting limits; and
- fixed resident scratch, stack and save-memory gates.

It does not own move legality, the damage formula, type effectiveness, status
rules, effect applicability or any other mechanical truth. Those are evaluator
correctness requirements and cannot be tuned away. Use `AI_CFG_*` names and
compile-time range/relationship checks; for example, the future 3x3 limits
cannot exceed the retained four AI/four player deep candidates. There is one
production configuration for `F_TRAINER_EXPERT_AI`, not per-trainer profiles.

## 5.5 Source ownership

- `include/trainer_ai.h` exposes only engine hooks and lifecycle functions.
- `include/ai_config.h` owns build-time tuning constants.
- `src/battle/trainer_ai.c` is the sole resident implementation unit. It owns
  the raw battle bridge, compact private value types, generated-table
  resolvers, evaluation, planning, prediction, switching, memory and fallback.
  Keeping one unit permits file-local sharing and avoids duplicated interfaces
  and call veneers under the overlay-130 budget. Keep those concerns in named
  function groups rather than splitting them unless a measured build proves a
  split has no size cost.

Pure strategic modules consume sanitized value types and do not include
`battle.h`. A static include/field audit enforces that selected-command fields
and unrestricted `BattleSystem *`/`BattleStruct *` pointers remain confined to
the bridge and lifecycle/save owners.

# 6. Exact-information, action-blind contract

## 6.1 Information the expert AI may know

At battle start and after relevant state changes, the sanitizer may copy:

- all player party species, forms and levels;
- exact four moves and current PP;
- exact current ability and held item;
- exact final calculated HP, Attack, Defense, Speed, Special Attack and Special
  Defense, including the effect of nature, IVs and EVs;
- current HP, major status, stat stages and supported volatile effects;
- types and public field/side conditions;
- whether each party member is alive and legally switchable; and
- the trainer party's equivalent data.

Do not copy raw IVs, EVs or nature when final calculated stats are sufficient.
Refresh mutable fields such as HP, status, PP, item, ability, form and stages.

## 6.2 Information the AI may not know

Before committing its own decision, the AI and every helper it invokes may not
read or infer:

- the player's selected command;
- the player's selected move slot;
- the player's selected switch destination;
- the player's selected target;
- future RNG results; or
- battle-controller fields whose only purpose at that time is to encode one of
  those selections.

The exact field denylist must be established during Phase 1 from the actual
battle controller. Command-blindness tests must vary those fields while
holding the sanitized state and AI RNG constant; the AI result must not change.

## 6.3 Sanitized value types

Illustrative compact view:

```c
typedef struct {
    u16 species;
    u16 move[4];
    u16 ability;
    u16 item;
    u16 hp;
    u16 maxHp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 spAttack;
    u16 spDefense;
    u8 pp[4];
    u8 stages[8];
    u8 type1;
    u8 type2;
    u8 form;
    u8 level;
    u8 partySlot;
    u8 flags;
    u8 semanticState; /* compact live activations such as Flash Fire */
    u32 condition;
    u32 condition2;
} AIMonView;
```

This is guidance, not a frozen ABI. Order fields deliberately and enforce a
compile-time size check. The resident planner should retain only active and
shortlisted views needed by a stage of the decision, not permanent duplicate
copies of two complete parties when compact derived descriptors are sufficient.
The view remains an information firewall even though caller and evaluator are
resident: strategic helpers accept sanctioned values instead of unrestricted
live battle pointers that could expose the selected command.

Move IDs alone are insufficient. Combine the existing move table's power,
type, split, accuracy, effect chance, priority, targeting and flags with a
compact AI semantics registry keyed primarily by `MOVE_EFFECT_*`. Use a sparse
move-ID override table only where one effect ID does not fully distinguish
legality or strategic behavior. Do not duplicate numeric move data already
available resident-side.

# 7. Actions, legality and evaluator coverage

## 7.1 Unified actions

```c
enum {
    AI_ACTION_MOVE = 0,
    AI_ACTION_SWITCH = 1,
};

typedef struct {
    u16 value;  /* move ID for MOVE; zero/reserved for SWITCH */
    u8 type;
    u8 index;   /* move slot 0..3 or party slot 0..5 */
} AIAction;
```

Generate every legal move and voluntary switch. ITEM does not exist in either
side's action space because Bag use is disabled. Forced Struggle and forced
replacement use dedicated legal paths.

## 7.2 Evaluator coverage

Unimplemented moves are outside the evaluator scope and must be filtered using
the same configuration/runtime rules as party construction and move learning.
Do not spend code or table space modelling them.

For implemented content, maintain the source-controlled
`documentation/TRAINER_AI_SEMANTICS_COVERAGE.md` report. Each move/effect is
classified as Exact, Generic, Conservative, Ignored, Unimplemented, Needs work
or Investigate. Exact and Generic entries document:

- a mechanics descriptor;
- a generic semantic handler or deliberate move-specific override;
- legality/failure, timing, cost and persistence behavior;
- damaging and secondary components where applicable; and
- a deterministic action-local score or outcome model.

Missing semantics do not fail an ordinary build. They are recorded as Needs
work or Investigate and prioritized from trainer/player reachability and battle
testing. Compile-time checks still validate the bounds and handler IDs of data
that is present. An `AI_EFFECT_FALLBACK` path scores only the affected action
with its documented conservative policy; it never randomizes, discards or
changes understood actions. Some effects may remain deliberately Ignored or
Conservative when bespoke logic would not change a useful tactical decision.

### 7.2.1 Generated-semantics build path

`data/trainer_ai_semantics.json` is readable source data. The ROM never parses
it. `tools/generate_trainer_ai_semantics.py` converts reviewed semantic classes
to compact runtime data. A normal dependency rule regenerates:

- `include/constants/generated/trainer_ai_semantics_generated.h`, containing only compact
  runtime mapping/recipe data and compile-time bounds; and
- no coverage gate. The Markdown coverage report is maintained separately so
  incomplete classifications remain visible without blocking normal builds.

The generated header is included exactly once by `src/battle/trainer_ai.c`.
It must not be included by public headers or multiple translation units because
its file-local tables would be duplicated. The evaluator exposes no table
pointers outside that implementation unit.

The generated runtime data contains:

1. sparse packed `u16` effect/class entries for the configured effect-ID range
   (the current base range is `0..405`);
2. sparse effect-ID lists for conditional secondary families such as flinch;
3. packed sparse move-ID class entries only where the effect ID and existing numerical
   move table do not fully determine behavior;
4. packed sparse ability/type entries; and
5. sparse consecutive-move transition entries.

Very small priority, status-prevention and held-effect families remain direct
shared comparisons until their cardinality makes a table smaller. This is a
representation choice only; it does not permit Pokémon- or moveset-specific
scoring code.

Do not duplicate power, type, split, accuracy, PP, effect chance, priority,
targeting or flags already present in the resident move table. At generation
time compare dense packed descriptors, effect-to-recipe indices and sparse
encodings; select the smallest representation that preserves complete
semantics, and report its linked `.rodata` cost. For reference, a frozen
two-byte entry for the current 406 base effects would cost 812 bytes before overrides,
but 812 bytes is an estimate, not a required representation.

### 7.2.2 Runtime semantic dispatch

For one evaluated move:

1. the sanitizer/bridge supplies its ID and existing numerical move data;
2. `AI_GetMoveSemantics()` maps the move effect to a generated shared recipe;
3. a move-specific override, if present, refines that recipe;
4. `AI_ApplyMoveSemantics()` dispatches a handwritten mechanics handler
   against the isolated simulated state; and
5. the planner values the resulting damage, status, stages, field changes,
   costs and persistence in the current action pair.

The generated data says what state transitions a move requires; handwritten
handlers implement those transitions; the planner decides whether the result
is useful. Coverage strings, JSON keys and the readable report are not linked
into release output.

Static source audit evidence from the rollback worktree establishes the
starting backlog, not the final generated gate:

- 232 expert trainer entries and 740 trainer Pokémon resolved to 400 distinct
  runtime-reachable implemented moves;
- none of those 400 were engine-unimplemented after applying runtime filtering;
- the removed legacy strategic evaluator fully modelled 129 of the 400;
- 48 status moves (47 effect families) had no strategic model; and
- 223 damaging moves (106 effect families) had ordinary damage modelled but
  their strategic effect semantics omitted.

The broader implemented move catalog contained additional gaps, so the tracked
report must eventually cover the actual player-obtainable move surface rather
than assuming expert-trainer coverage is sufficient. Resolve the recorded
shared-learnset/form caveat as part of that work. These counts are a backlog
baseline, not a build or release gate.

## 7.3 Strategic semantics and dynamic roles

Do not assign a permanent role to a species. Derive independent capability
scores from its exact current moves, stats, ability, item, health, field,
teammates and opponent branches. At minimum derive defensive-pivot, immediate-
attacker, setup, recovery/support and sacrifice/tempo capabilities. A Pokémon
may satisfy several roles; the common resulting-position screen lets those
capabilities compete without fixed role labels. Shortlisting never overrides
the unified final action utility.

The semantics registry describes what an action does. The planner decides
whether that result is useful. For example, a Defense drop descriptor supplies
the stage delta and switch-removal behavior; the planner determines whether
Leer plus stronger attacks beats immediate attacks after survival and likely
switching are included.

# 8. Tactical evaluation and caching

## 8.1 Tactical primitives

Use signed 16-bit utility with values clamped away from a dedicated invalid
sentinel. Use Q8 probabilities `0..255`; do not use floating point. Normalize
HP exchange so 100 utility points represent one complete HP bar. Preserve raw
HP damage as well, because KO thresholds and turns-to-KO must not be inferred
from normalized percentages alone.

The core represents expected/conservative damage, all sixteen standard damage
rolls when a KO threshold matters, accuracy, priority, effective speed, fixed
and variable power, current HP, status/stage legality, recovery, residuals,
hazards, screens, weather/field state and reachable item/ability interactions.
The production battle RNG is never consumed by evaluation.

### 8.1.1 Legality and effective move

Before scoring, reject an action that cannot execute because of PP, disabled or
locked state, trapping/switch legality, target legality, status, Taunt-like
restrictions or another reachable mechanic. Forced Struggle is a dedicated
legal action. Impossible actions receive the invalid sentinel, not a low
preference score.

For a legal move resolve split, effective type, variable/fixed power, priority,
accuracy rules, target and any move-specific precondition through existing
numeric data plus the generated semantic recipe. This step also identifies
whether critical, multi-hit, recoil/drain, charge/recharge, pivot, phasing or
other non-ordinary handling is required.

Treat mechanical applicability, probability and tactical payoff as separate
questions. A legal move whose effect cannot change the concrete action-pair
state receives the invalid sentinel. Otherwise, use the fork's actual numeric
data and condition state to weight successful and failed outcomes; never invent
a generic failure percentage for an evaluator gap. Accuracy, conditional
success and action-denial probabilities compose without consuming production
battle RNG. Unsupported semantics retain only their documented conservative
action-local score.

Consecutive Protect-family rules have one shared source definition used by the
battle controller and evaluator. Protect/Detect and Endure use the actual
`1, 3, 9, 27, 81, 243, 729` denominators. Fork-specific exceptions such as
Quick Guard, Wide Guard, Mat Block and Crafty Shield do not inherit declining
odds. Endure has distinct payoff: it is useful only against a damaging branch
that would otherwise be lethal, and successful survival is valued separately
from completely blocking a move.

`MOVE_EFFECT_CURSE` branches on the user's actual types. Ghost Curse is invalid
against an already cursed or substituted concrete target and weighs the 50%
maximum-HP sacrifice against switch-discounted residual value. Non-Ghost Curse
evaluates the resulting Attack, Defense and Speed changes together. A capped
or tactically neutral component is omitted; it does not discard other changes
that still apply. Combined-stage moves charge the incoming/setup-turn cost
once.

### 8.1.2 Damage and KO calculation

For ordinary damage, calculate effective offensive and defensive stats from
the exact snapshot and stages, select the physical/special path, then apply the
engine's integer base formula:

```text
base = (((2 * level / 5 + 2) * power * attack / defense) / 50) + 2
```

Use audited 32-bit intermediates and reproduce modifier order where integer
rounding changes the result. Apply critical-stage rules, burn/comparable
penalties, STAB, type immunity/effectiveness, weather, screens, ability, held
item and move-specific modifiers. Reachable fixed-damage and variable-power
moves use semantic handlers rather than fabricated ordinary power.

When a KO threshold matters, calculate the sixteen 85–100% damage rolls and
derive minimum, maximum, expected damage and exact roll-count KO probability,
multiplied by effective hit probability. Fast screening may use bounded
expected/conservative summaries; retained pairs use the threshold result.
Immunity is resolved before damage and remains an explicit catastrophic
stay-branch outcome where appropriate.

The current compact resident pass does not yet enumerate those sixteen rolls.
It calculates one accuracy-adjusted damage estimate, caps only the HP actually
lost when producing primary utility, and retains the uncapped estimate as a
compact signed value, saturated only at the representation limit. Pressure is
not overkill utility: it is consulted only when two actions have exactly the
same final primary score. This prevents two
apparently lethal attacks from collapsing to an arbitrary move-slot choice
while preserving the utility scale and resident budget.

### 8.1.3 Deterministic action-pair transition

Evaluate a trainer action and one specific player action in an isolated compact
state:

1. apply voluntary switches before ordinary moves, including hazards and
   switch-triggered state;
2. if both switch, evaluate the resulting matchup without inventing an attack;
3. otherwise order moves by priority, effective Speed, Trick Room and supported
   modifiers;
4. treat known unequal Speed as deterministic and represent only a true Speed
   tie or an explicitly stochastic order mechanic as weighted outcomes;
5. apply the first action's miss/hit/effect branches;
6. omit the second action if its user fainted or became unable to act;
7. apply the second action; and
8. apply supported end-of-turn recovery, status, weather, seed and comparable
   residual effects.

A player switch means the trainer's selected move is evaluated against the
specific incoming Pokémon. A trainer switch means the player's selected move
is evaluated against the incoming trainer Pokémon. The simulator mutates only
its local state and never executes production battle scripts.

The resident implementation represents consecutive mechanics with one generic
active move state: forced move, previous move, forced progress, repeat progress
and flags. Move IDs are mapped by generated semantics to normal,
force-repeat-scaling or repeat-scaling transitions. It does not add a field for
each exceptional move. Rollout and Ice Ball use the forced scaling transition;
Fury Cutter uses the freely selectable repeat-scaling transition. Only active
simulated battlers carry this state, so party size does not multiply it.

### 8.1.4 Common utility scale

The immediate HP component is:

```text
damage dealt  =  256 * playerHpLost / playerMaxHp
damage taken  = -256 * trainerHpLost / trainerMaxHp
```

Add positional changes for fainting, remaining usable party members, action
denial, status, stages, hazards/screens, recovery, residuals, switching tempo
and the resulting matchup. Avoid flat bonuses where a mechanic can instead be
valued through the damage, survival or actions it changes. For example, a
Defense boost is worth its reduction in future incoming damage and increased
survival, not an unconditional `+Defense` constant. KO/party terms still exist
because equal HP percentages do not make losing a Pokémon strategically
neutral.

Each retained outcome supplies immediate utility, resulting state, survival
margin and enough flags for coverage/catastrophe classification. The analytic
plan evaluator in Section 9 supplies the discounted continuation; it does not
double-count the first-turn HP/state delta.

Final deterministic selection is lexicographic. Higher primary utility always
wins. If primary scores are exactly equal, prefer the action with greater
uncapped active-target damage pressure, then retain stable action order. The
pressure tie-break does not replace concrete switch-branch coverage: pairwise
utility still decides whether a safer coverage move or switch is strategically
better.

## 8.2 Conditional secondary effects

Secondary effects are probabilistic state transitions, not flat move bonuses.
For each AI-action/player-branch pair calculate the applicability and
probability of each effect from:

- hit probability and the move's `effectChance`;
- effective priority/speed order, including Trick Room and supported modifiers;
- whether the target survives and still has an action to lose;
- target state, immunity and effect-prevention ability/item rules;
- effect amplification or suppression such as Serene Grace or Sheer Force;
- whether the player attacks, uses support or switches; and
- whether the effect benefits the current turn, future turns or the user's
  resulting state.

Classify timing explicitly. Action denial such as flinch has value only when
the user acts first and the surviving target still has an action. Current-turn
mitigation such as an Attack drop has an immediate component only before the
attack, but may retain future value afterward. Persistent status retains future
value whenever it is legal and the target survives. User-side boosts may retain
value even when the target is KOed.

Fast scoring uses bounded expected value. Retained expensive pairs may branch
into effect/no-effect/miss transitions and weight their resulting utilities
without consuming production battle RNG. For example, ordinary faster Rock
Slide starts from `90% * 30% = 27%` flinch probability before modifiers;
slower Rock Slide, a KO, or a player switch gives the flinch component zero
value while preserving its damage result.

Random-selection actions such as Metronome, Assist or Sleep Talk receive a low
but positive bounded score only when legal and capable of producing an action.
They normally rank below reliable useful choices, but variance/desperation
weighting may admit them when conventional lines are unlikely to avoid defeat.
Do not expand every possible called move into the main search. A move that
cannot function in the current state is invalid, not positive.

## 8.3 Per-decision cache

Exact player data removes belief expansion but not repeated arithmetic. The
resident planner uses small fixed per-decision caches keyed by sanitized
view/action identity:

- damage for retained attacker/target/move combinations;
- incoming threat for concrete player branches;
- effective speed/priority relationships;
- hazard damage for shortlisted switches; and
- post-stage best damage used by plan summaries.

Use compact stack or tightly bounded battle-owned scratch where lifetime
requires it. Do not add a large permanent matrix to overlay-130 BSS.

The command hook computes one complete decision and the move hook consumes it.
Cache identity is the turn, battler, active party slot and decision mode. Do
not add a `stateFingerprint`: no battle mechanics should execute between the
paired hooks, a 16-bit checksum can collide and hashing adds unnecessary code.
If Phase 1 proves a legal same-turn/same-slot mutation path, introduce an
explicit monotonically controlled decision generation instead. Cache scratch
is overwritten at the next decision, cleared at battle teardown and never
persisted.

## 8.4 Immunity and tactical-safety rules

- A damaging move that deals zero to the current target records a catastrophic
  stay-branch outcome, not a neutral zero.
- Such a move may remain a specialized prediction candidate only when the
  switch branch is sufficiently credible and no safe alternative satisfies the
  configured coverage/regret constraints.
- If a safe legal action covers the current target and the likely switch, an
  immunity gamble cannot enter the robust policy or a calculated mixed
  strategy.
- Prior read outcomes influence branch probabilities and mixup breadth through
  learned cases; do not apply an automatic identical-read repeat penalty.

# 9. Multi-turn plan evaluation

The AI needs longer-horizon judgment without a deep search tree. Each strategic
action produces a compact deterministic plan summary.

## 9.1 Turns-to-KO comparison

For ordinary repeated attacks:

```text
attackTurns = ceil(targetHp / bestCurrentDamage)
```

For a one-turn setup or stat change:

```text
setupTurns = 1 + ceil(targetHp / bestPostSetupDamage)
```

Compare more than turn count:

- opposing attacks received before the expected KO;
- whether the trainer survives the setup turn and reaches the payoff;
- move accuracy and status/setup success;
- priority/speed changes that alter how many opposing attacks occur;
- healing, residual damage and supported field effects;
- probability and consequences of either side switching; and
- value of the resulting position after the target faints or switches.

Example: if Tackle takes six uses and Leer makes Tackle a four-hit KO, the Leer
plan takes five trainer turns total. Leer saves one turn, not two. It is useful
only if the trainer survives, the target is likely to remain long enough and
the alternative positional consequences are favorable.

## 9.2 Self boosts versus target drops

Do not apply an unconditional self-boost bonus. Model persistence:

```text
target-drop value =
    probability target stays * payoff against lowered target
  + probability target switches * tempo/hazard/switch-in payoff

self-boost value =
    payoff against current and replacement targets
  * probability the trainer remains active
```

Consequences:

- Self boosts normally retain value when the player switches.
- Target drops have full persistence against the player's last Pokémon or a
  trapped target.
- A player switch clears its drops but consumes a turn, may take hazard damage
  and may expose the replacement to the trainer's selected follow-up attack.
- A trainer self boost is also temporary if the trainer is likely to switch or
  be forced out.
- Repeating setup at a cap or after its marginal payoff disappears is invalid.

## 9.3 Plan summary

An illustrative plan record contains:

```c
typedef struct {
    s16 immediateUtility;
    s16 futureUtility;
    s16 survivalMargin;
    u8 turnsToKo;
    u8 incomingActions;
    u8 persistence;
    u8 flags;
} AIPlanSummary;
```

The exact representation should be smaller if the same semantics can be
derived. The planner evaluates only the best follow-up action for a simulated
state; it does not branch over another complete player response after that.

## 9.4 Plan generation and survival accounting

For each promising first action, apply bounded state transitions and retain
effect ownership, duration and removal rules rather than assigning a flat
setup bonus. Relevant continuations include:

- repeat the best direct attack;
- setup once or repeatedly while another use improves the projected result,
  then use the best post-setup attack/support continuation;
- apply a target drop/status, then exploit it;
- recover, then attack/setup if survival changes;
- use support/screens/hazards, then exploit their remaining turns;
- switch, then take the incoming Pokémon's best plan; and
- use a pivot/phasing action, then value the resulting matchup.

Cap summaries at `AI_CFG_PLAN_TURN_CAP`. For each template calculate trainer
turns to payoff, opposing actions received before payoff, expected and
conservative trainer HP remaining, accuracy/effect success, residuals, likely
switch clearing, and resulting position after a KO or switch. Recovery receives
value only for HP actually restored and the additional actions/survival it
creates; setup that cannot survive to use its benefit is rejected.

This also applies defensively. If the active player Pokémon can deal only
5–10%, the trainer compares using that low-pressure window for self setup with
switching immediately to a reserve that dies in three hits. Setup is favored
only when the current trainer survives the setup sequence, retains the boost,
and its post-setup damage/speed/survival against credible current or reserve
targets exceeds the best switch/direct plan. The evaluator does not award
setup merely because the current incoming damage is low.

Classify strategic state by where it persists:

- side-duration effects such as Light Screen, Reflect and Tailwind survive
  switching and retain their exact remaining turns; known removal such as
  Brick Break is a concrete player response with its own opportunity cost;
- user-bound boosts survive player switches but disappear when the trainer
  leaves;
- target-bound drops, seed and volatile control may deliver immediate value
  and switch-forcing tempo even when the player can clear them next turn; and
- entry hazards are valued over the actual living reserves, their HP/types,
  entry cost, removal options and probability of entering.

For a target-bound effect, value the guaranteed current-turn change, expected
retained turns and the position created by a forced switch. A healthy free
counter-switch sharply reduces future drop value; depleted reserves, hazards,
fainted counters, trapping and last-Pokémon states increase it. Persistent
side effects are valued through the damage, action order and viable team plans
they change, not merely because they are difficult to remove. The planner is
receding-horizon: it evaluates the resulting real state again next turn rather
than blindly committing to the remainder of a setup sequence.

# 10. Trainer switching and post-KO selection

## 10.1 Cheap screen of all reserves

The resident planner scores every legal reserve using inexpensive exact-state
descriptors:

- hazard damage and survival after entry;
- expected incoming damage, resistance or immunity;
- current HP and status;
- speed and revenge-KO pressure;
- best direct attack;
- recovery, setup, status, screens or other support value; and
- resulting utility against every retained player response.

Do not add a mandatory action after entry, recent-switch penalty, switch
cooldown or speculative loop breaker. Switching competes normally on every
turn. If repeated switching is ever observed as a real pathology, investigate
the prediction and state evaluation that makes each switch optimal before
adding any special-case restriction.

## 10.2 Admit the two strongest resulting positions

Use one shared screen combining entry survival, best immediate pressure,
expected opposing pressure and remaining HP. These components naturally admit
defensive pivots and immediate attackers without three parallel role-winner
arrays. A support Pokémon can still qualify through its safe resulting
matchup, but the cheap shortlist does not rescan all of its status moves;
their exact utility is evaluated when the Pokémon becomes active. This is a
deliberate resident-size tradeoff with small expected player-visible impact.

Retain at most the two highest-value distinct reserves and fully evaluate them against every
concrete player branch during the first-turn pass. At most two switch actions
enter the final searched action set, leaving room for move actions.

The applied shortlist keeps the two best unified screening scores while
walking every legal reserve. An ordinary switch-in must
survive predicted entry damage and improve the common resulting-position score
by `AI_CFG_SWITCH_MIN_IMPROVEMENT`, except when the active Pokémon is already
expected to faint. This is matchup eligibility, not a recent-switch cooldown.

Post-KO replacement uses the same role and plan model but has no “switch costs
this turn” penalty. It enters through the guarded seam in Section 3.4 and has
its own cache mode/generation.

## 10.3 Engine switch handoff

For a legal voluntary switch, the resident integration bridge writes the
validated party slot to `BattleStruct::ai_reshuffle_sel_mons_no[battler]` and
returns `SELECT_POKEMON_COMMAND`. This field is the existing HGSS handoff for
the AI's selected reserve; writing it does not perform the switch or mutate the
active slot. The ordinary battle controller consumes it and executes normal
switch processing.

Validate the slot immediately before writing: it must exist, be alive, not be
the active slot, remain switch-legal and match the cached selected action.
Post-KO replacement uses the separate guarded selector in Section 3.4 rather
than pretending to be a voluntary turn switch.

# 11. Player action prediction

The AI knows exactly which actions the player possesses but not which action
the player chose. Possession inference is removed; choice prediction remains.
Prediction produces a calibrated distribution over concrete actions, not one
unqualified guessed action. Bag actions are absent because Bag use is disabled.

## 11.1 Candidate generation

Generate:

- every legal player move;
- every legal player switch destination; and
- forced actions where applicable.

Keep each move and each destination distinct during immediate evaluation. In
ordinary singles this is at most four moves plus five switch destinations, so
discarding branches before cheap coverage analysis provides little benefit.
Strategic classes support learning but never replace concrete move/switch
identities in the current decision.

## 11.2 Visible trainer threat envelope

Avoid circularly predicting the player against the trainer's not-yet-selected
action. First construct a small visible threat envelope from cheap trainer
scores: strongest immediate/priority attack, credible coverage, credible
setup/support and best obvious switch. Score player choices against that set,
representing what a skilled player could reasonably anticipate from public
state rather than leaking the eventual hidden trainer selection.

A player switch is scored by its survival against credible trainer attacks,
entry hazards, speed, threat back, recovery/setup opportunity and value of
preserving the active Pokémon. Do not choose a predicted destination merely by
highest HP.

## 11.3 Probability model

Each concrete action receives:

```text
weight = rational tactical weight
       * recent-battle behaviour adjustment
       * persistent abstract-behaviour adjustment
```

The rational component considers:

- KO and damage value;
- priority, survival and speed;
- setup/recovery/status value;
- switch matchup, hazard cost and remaining HP;
- whether the active Pokémon is threatened with a KO;
- preserving a valuable low-health Pokémon; and
- whether the choice covers several credible trainer threats.

Apply the eight recent-battle cases and 32 abstract persistent cases only after
the rational baseline. Convert adjusted scores to Q8 probabilities with a
small fixed-point weighting function owned by `ai_config.h`. Give every viable
action a nonzero smoothing floor, normalize to 255 and prevent one observation
from creating certainty. Calibration changes distribution sharpness; it never
invents or removes a legal action.

The resident baseline assigns move and switch probability mass separately,
distributes each mass across its concrete actions by tactical weight and gives
integer-normalization remainder to the strongest member of that group. A
mechanically forced move is the sole action with probability 255. Later
calibration work adjusts these weights rather than replacing this legal-action
distribution.

## 11.4 Cheap complete response scan

Before shortlisting trainer actions, cheaply scan every legal trainer action
against every concrete player action—normally no more than roughly 9x9 in
singles. Stream the results rather than retaining a matrix. For each player
branch identify its best trainer reply, safest reply, specialized high-upside
reply and consequence if uncovered.

Build the four-action trainer shortlist from distinct needs, then deduplicate:

1. best expected-value action;
2. best robust/minimum-regret action;
3. best response to the highest-probability branch;
4. best response to the most dangerous credible branch; and
5. best credible switch or strategic action.

This prevents an unconditional top-four score from deleting the one coverage
move or pivot required to answer the predicted switch.

## 11.5 Reaction and anti-exploitation

After the player's actual action becomes public, resolve the previous
prediction before constructing the next distribution. Update the matching
recent case and bounded abstract persistent tendency using only information
already revealed by play. Examples include switching when threatened by a KO,
using an immunity pivot, preserving low HP, recovering at a threshold, setting
up into low pressure or repeatedly countering the trainer's apparent threat.

Variation is not a presentation feature. Remove dominated actions and select
the best prediction/coverage policy. A mixed strategy is permitted only among
non-dominated actions when its calculated distribution reduces exploitability
or responds to evidence that the player is counter-reading the trainer. Its
weights come from player-action probabilities and pairwise payoffs, not an
arbitrary near-best randomizer. Do not apply a blind repeat or switch penalty:
clearly superior actions may repeat.

# 12. Pairwise evaluation, coverage and selection

## 12.1 Bounded search

- Generate and probability-score every concrete legal player move/switch.
- Run the cheap complete trainer-action/player-action response scan.
- Retain at most four role/response-distinct trainer actions, including a move
  and switch when each class has a credible candidate.
- Calculate full first-turn pairwise utility for all four retained trainer
  actions against every concrete player branch (normally at most 4x9).
- Select at most four highest-relevance player branches as deep candidates and
  apply expensive future-plan evaluation to at most a relevant 3x3 subset.
- Mechanics intentionally approximated in the bounded future transition retain
  their complete direct pairwise value; this is not permission for an unknown
  move effect.
- Stream results into per-action summaries; do not retain a production matrix
  after the decision.

## 12.2 Coverage semantics

For each AI action and player branch, calculate:

- direct/future utility;
- regret relative to the best available response to that branch; and
- whether the outcome meets both minimum utility and maximum regret.

```text
expected(a) = sum(probability(b) * utility(a, b))
regret(a,b) = utility(best reasonable response to b, b) - utility(a,b)
robust(a)   = expected(a)
            - riskWeight * expectedDownside(a)
            - regretWeight * maximumCredibleRegret(a)
```

The applied first resident pass implements the probability-weighted portion
without an absolute-worst shortcut:

```text
expectedDownside(a) = sum(p(b) * max(0, expected(a) - utility(a,b)))
score(a) = expected(a) - riskWeight * expectedDownside(a)
```

Every nonterminal first-turn pair then receives the same discounted next-turn
best-action evaluation. This equal horizon is applied to all four move/switch
pair shapes; no action receives extra future depth merely because it is a
switch. Forced consecutive-move state is carried only on the hit branch.

Utilities for at most nine concrete player branches are retained only in one
reused 18-byte stack array while scoring an action. Regret and mixed-strategy
selection remain later tuning work; deterministic maximum score is used in
the meantime.

Forced continuations and speculative continuations use separate discounts. A
successful Rollout switch-in branch carries the lock and progression into the
following comparison; a miss clears it. If the next trainer Pokémon is faster
and KOs first, the forced attack contributes no invented damage. An unlocked
opponent receives its normal switch choices, including after a trainer
sacrifice, so replacement pressure is not treated as a guaranteed revenge KO.

Use the cheap complete response scan to establish the best reasonable response
for regret; comparing only the four retained trainer actions would understate
regret when shortlisting omitted a necessary counter.

Covered mass is the sum of probabilities for covered branches. Selection:

1. If one or more actions meet `robustCoverageTarget`, only those actions are
   robust-eligible.
2. Remove actions dominated across all credible branches. Applicable
   secondary effects, accuracy, priority, PP, contact consequences and switch
   coverage participate in dominance, so an otherwise-equivalent attack with
   a usable flinch dominates one with no compensating benefit.
3. Select the best robust action unless configured payoff clearly justifies a
   specialized prediction.
4. If no action meets the coverage target, compare weighted value, worst
   credible outcome and regret explicitly.
5. An immunity or other catastrophic credible branch cannot be hidden by a
   small coverage bonus.
6. Sample only when a calculated non-dominated mixed strategy improves
   prediction warfare or reduces exploitability. An exact tie may use a stable
   deterministic tie-break when no mixed strategy is justified.

Match position selects configuration bands rather than changing mechanics.
When comfortably ahead, require high coverage and strongly veto catastrophic
credible or low-probability outcomes. In an even battle, allow calculated
mixups. When behind, accept more prediction risk. When conventional lines are
very unlikely to avoid defeat, admit narrow hard reads and low-positive
random-selection move effects whose upside can recover the game. This does not
authorize random selection among ordinary AI actions.

A specialized prediction is eligible only when its probability-weighted
payoff exceeds the best robust line by the configured margin, its uncovered
risk fits the current position band and the supporting behavioural evidence is
sufficient. Thus an Electric move into an active Ground Pokémon is normally
rejected when safe coverage protects a winning position, but can be selected
as a deliberate switch read when the predicted destination/payoff and losing
position justify the risk.

This corrects the discarded overlay evaluator, which accepted a
`coverageTarget` field but did not use it as an eligibility constraint.

# 13. Prediction commitments and memory

## 13.1 Commitment

A prediction commitment exists only when the chosen action intentionally
sacrifices meaningful branch coverage for a particular read. A robust action
creates no commitment.

```c
typedef struct {
    u16 chosenActionKey;
    u16 coveredBranchMask;
    u8 active;
    u8 situationClass;
    u8 predictedClass;
    u8 reserved;
} AIPredictionCommitment;
```

Resolve the commitment after the player's actual action is known, before the
next decision uses the result. Resolve the final pending commitment at battle
end.

## 13.2 Outcome classification

| Actual outcome | Classification |
|---|---|
| Chosen specialized action covered the actual branch | AI read win |
| Chosen action failed, and another reasonable action could have covered it | Player read win |
| No reasonable action could cover it | Neutral/unavoidable |
| Robust action was chosen | No read |
| Actual low-probability legal action was counterable | Counterable surprise |
| Low-probability action was not counterable | Neutral surprise |

Exact loadout knowledge removes the old “newly revealed unknown move” class.

## 13.3 Recent-battle cases

Retain a small fixed recent-history window for the current battle, initially up
to eight compact situation/action/outcome entries. It supports immediate
adaptation and is cleared before an unrelated trainer battle. It does not apply
an automatic penalty to repeating a failed read; repetition changes only when
the learned action distribution, match position, coverage or mixup value makes
another choice better.

## 13.4 Persistent cross-battle cases

Persist 32 abstract player-behaviour cases shared across battles. Each case is
targeted at 6–8 bytes and may encode only strategic classes such as situation,
predicted action, actual action, result, confidence and recency. It may not
encode trainer, species, form, move, item, ability, party-slot or matchup
identity. Examples include switching when threatened by a KO, attacking into a
setup opportunity, preserving a low-health Pokémon or selecting an immunity
pivot.

A small versioned header may retain aggregate `readBalance`,
`surpriseTendency`, count and replacement cursor fields. The cases occupy
192–256 bytes and the complete logical store targets at most 272 bytes before
measured save-block alignment. Save
layout, serialization, checksum behavior, initialization, migration and corrupt
data handling must be reviewed before enabling persistence. Invalid or unknown
versions reset only this AI store to neutral defaults.

Cases adjust action-probability sharpness, robust-versus-specialized tolerance,
low-probability mass, desperation variance and safe mixup breadth. They do not
force a particular counteraction or impose a repeat ban.

Resolve and update the in-memory working copy after an action is revealed, but
write the versioned persistent store through the audited save owner rather than
performing ad-hoc save writes from the decision hot path. Battle end resolves
the final outstanding commitment before the store is finalized. Save failure
or invalid version resets/ignores only AI calibration and never blocks battle
progress.

# 14. Resident placement and transient approval gate

The production target is entirely resident in overlay 130. A transient overlay
is not part of the approved architecture. Running out of resident capacity is a
stop condition, not authorization to create one.

## 14.1 Resident fit enforcement

- Measure final `output_battle.bin`, every allocated section end, AI object
  text/data/BSS, largest symbols, stack and battle-heap workspace at each phase.
- Preserve at least 500 bytes of linked overlay-130 headroom.
- Reduce duplicated mechanics, tables, literals and workspace before proposing
  another placement; do not weaken correctness or coverage silently.
- If the complete design still cannot fit, stop implementation and present the
  measurements, attempted reductions, remaining options and expected transient
  runtime cost to the user.

## 14.2 Required explicit approval

Before any transient work, ask the user explicitly. General authorization to
implement the TDD, fix the AI, continue work or build the ROM is not approval.
Without a specific affirmative answer, do not reserve an overlay ID, add a
linker region/image, add loader or unload code, create a fixed entrypoint, or
move any evaluator function or table into the individual-overlay region.

## 14.3 Conditional safety contract after approval

Only if a later TDD amendment and implementation are explicitly approved:

- Reserve a unique ID, expected to be 150, only after a fresh uniqueness check.
- Link at `0x023C0400` with region length `0x3C00`.
- Put a tiny entry veneer at the origin and assert its exact address.
- Link as a zero-import image: no `rom_gen_battle.ld`, shared helper object or
  unresolved external symbol.
- Provide private arithmetic/memory helpers required by transient code.
- Poison overlay loader/unloader, task/yield APIs and forbidden raw executable
  addresses in the transient source environment.
- Run a static validator and require an empty undefined-symbol list.

These constraints make accidental resident/helper calls a build failure. They
do not replace source review of inline assembly or raw addresses.

## 14.4 Conditional resident wrapper

One resident wrapper:

1. confirms expert singles and a non-forced decision;
2. verifies overlay ID/capacity metadata and `CanOverlayBeLoaded()`;
3. confirms an available tracking slot without unloading another owner;
4. loads the overlay and records whether loading succeeded;
5. calls the fixed entrypoint synchronously;
6. unloads exactly once after a successful load;
7. validates result magic, counts, indices, action identities and commitment;
8. uses resident tactical fallback on any preflight/result failure.

Never add overlay 150 to an automatic priority list that unloads another
overlay. If the region is occupied, skip the planner and preserve the owner.

## 14.5 Conditional critical prohibitions

While executing at `0x023C0400`, transient code must never:

- call `CalcBaseDamage` or another helper that loads an individual overlay into
  the same region;
- load or unload any overlay;
- yield, schedule a task, retain a callback or return while work is pending;
- retain pointers to transient code, literals, globals or stack data; or
- mutate live battle/party state.

The request must contain all exact sanitized descriptors needed by the
planner. The result contains only action IDs, utility/coverage summaries,
prediction metadata and calibration evidence.

# 15. Performance requirements

## 15.1 Required instrumentation

Development measurement must separately time:

- exact snapshot refresh;
- legal action generation;
- tactical primitive/cache construction;
- move-semantics and conditional-secondary evaluation;
- trainer switch screening;
- player distribution and plan/search evaluation;
- final validation/sampling; and
- total decision time.

The discarded attempt's visible pause cannot be attributed to overlay loading
alone: its overlay was only 676 bytes while resident code repeatedly performed
damage, personal-data, threat, reserve and pair calculations. The resident
design removes the load entirely and must still eliminate repeated mechanics
work through the per-decision caches.

## 15.2 Runtime gates

- Ordinary decisions target at most two video frames total.
- Forced actions skip unnecessary planning.
- Command and move hooks evaluate at most once through the decision cache.
- No initial “obvious decision” fast path is required by v0.6.
- If runtime exceeds the target, optimize duplicated primitives and reduce the
  expensive 3x3 subset before removing complete immediate player-action
  coverage.
- Runtime pressure does not authorize a transient overlay; any such proposal
  follows the explicit approval gate in Section 14.

# 16. Decision pipeline

```text
AI_ThinkExpertSingles
    reject unsupported format/profile -> original HGSS AI
    resolve previous prediction commitment if actual action is available
    reuse valid command/move cache
    refresh exact sanitized state
    enumerate legal AI/player moves and switches
    build compact Pokémon, move and field descriptors
    build compact tactical caches
    cheaply screen all trainer reserves
    retain the two best distinct reserve positions
    generate every legal player move/switch and its probability
    build the visible trainer threat envelope
    cheaply scan every legal trainer action x player action
    retain every legal move plus up to two highest-position switches
    fully evaluate retained actions x every concrete player branch
    deeply evaluate only the relevant 3x3 future subset
    calculate plans, coverage, regret and ranking
    validate the selected action
    remove dominated actions and select the best prediction/coverage policy
    sample only if a calculated mixed strategy is justified
    cache decision and any genuine prediction commitment
    write command/move/switch fields expected by HGSS
```

Post-KO selection uses a dedicated mode and cannot reuse an ordinary cached
decision.

# 17. Phased implementation plan

## Phase 0 — Audit the rollback and establish a controlled baseline

- Review the applied
  `documentation/TRAINER_AI_LEGACY_ROLLBACK_CHANGELIST.md`; the source rollback
  must remain surgical rather than a blind whole-commit revert.
- Confirm both the discarded v0.4 attempt and the committed legacy runtime are
  absent while unrelated changes, `F_TRAINER_EXPERT_AI` trainer assignments
  and the general move-split fix remain.
- Record the exact source revision and dirty state.
- Build the clean original-HGSS-AI baseline only when explicitly authorized.
- Measure `output_battle.bin`, allocated section ends, AI object text/BSS,
  largest symbols and exact 500-byte headroom.
- Verify the legacy hooks and runtime references are absent. Retain their
  verified addresses as documentation for Phase 1.
- Generate the initial implemented/reachable move-semantics coverage inventory,
  applying the engine's unimplemented-move filters.
- Create a regression inventory from Section 3.

**Exit:** baseline behavior and all memory budgets are measured rather than
estimated.

## Phase 1 — Integration shell and command-blind boundary

- Add the singles/unsupported-format gate before custom state access.
- Audit every `BattleStruct` allocation/clear and assembly size assumption,
  append the bounded `AIExpertBattleState`, and prove existing member offsets
  remain unchanged.
- Reintroduce command/move wrappers at the verified hook sites, original
  fallback, a new battle reset and active-slot cache protection.
- Reintroduce the guarded post-KO seam.
- Establish the exact selected-command field denylist.
- Implement legal deterministic fallback decisions.

**Exit:** moves, voluntary switches and post-KO replacements can be commanded
without reading the player's selected action.

**Resident size checkpoint required.**

## Phase 2 — Exact sanitized state

- Replace natural-move/ability/item/stat beliefs with exact compact snapshots.
- Copy final stats rather than raw EV/IV/nature.
- Refresh mutable loadout/state fields centrally.
- Add compile-time structure limits and command-blindness tests.

**Exit:** no possession inference or repeated personal-data lookup is required
inside evaluator loops.

**Battle-heap state, resident BSS/code and stack checkpoints required.**

## Phase 3 — Resident tactical core and coverage foundation

- Implement exact damage, KO, immunity, speed, priority, accuracy, recovery and
  status legality.
- Add `include/ai_config.h` with one production configuration, compile-time
  range/relationship checks and no configurable mechanical truths.
- Define the compact semantics registry, its single-translation-unit generated
  header inclusion, sparse move overrides, dynamic role capabilities and
  resident per-decision primitive caches.
- Implement the integer/Q8 utility model, sixteen-roll KO thresholds, ordered
  action-pair transitions and streamed outcome summaries from Section 8.
- Maintain the nonblocking semantics coverage report for implemented moves
  reachable by an expert trainer or player; ignore engine-unimplemented moves.
- Retain an action-local defensive fallback and record incomplete or deliberate
  conservative handling in the report without blocking ordinary builds.
- Make immunity outcomes tactically catastrophic where appropriate.

**Exit:** obvious KOs, immunities and legal tactical choices order correctly.

**Resident code/data/BSS, battle-heap scratch and stack checkpoint required.**

## Phase 4 — Complete move semantics and probabilistic outcomes

- Implement reusable handlers for all reachable variable power, fixed damage,
  multi-hit, recoil/drain/sacrifice, charge/recharge/locking, priority,
  conditional, reactive, status, stage, recovery, field, trapping, phasing,
  pivoting, copy and random-selection mechanics.
- Implement conditional secondary-effect applicability, probability and
  effect/no-effect/miss outcomes without production battle RNG.
- Give legal random-selection actions low positive desperation-aware value;
  invalid instances remain invalid.
- Add narrow move-specific overrides only where the effect ID is insufficient.

**Exit:** reachable gaps are classified in the Markdown report, high-impact
Needs-work entries are prioritized, and no fallback can affect another action.

**Resident size checkpoint required. If the hard gate is exceeded, stop and
ask before any transient-overlay work.**

## Phase 5 — Switching

- Add hazard/survival screening for every reserve.
- Score every reserve on the common resulting-position scale and retain two.
- Admit up to two switches to final scoring.
- Permit switching immediately after any entry and rely on ordinary
  prediction/state utility rather than loop penalties or cooldowns.
- Add voluntary-switch and post-KO regression scenarios.

## Phase 6 — Multi-turn plans

- Implement analytic attack/setup/drop/recovery/support plan summaries.
- Account for turns to KO, incoming actions, survival and effect persistence.
- Model player and trainer switching when valuing self boosts and target drops.
- Retain duration/removal semantics for screens, Tailwind and hazards; include
  immediate drain/tempo for seed and other target-bound effects.
- Permit repeated setup and side-effect-to-switch sequences while each bounded
  continuation improves the projected state.

**Exit:** relational scenarios such as Leer plus four Tackles versus six
Tackles produce positionally sensible choices.

## Phase 7 — Prediction, coverage and bounded planner

- Generate smoothed probabilities over every exact legal player move/switch
  using the visible trainer threat envelope.
- Cheaply scan all trainer/player action pairs and use expected, robust,
  highest-probability-counter and dangerous-branch-counter needs to construct
  the trainer shortlist.
- Integrate tactical, switch and plan modules into full first-turn evaluation
  of four retained trainer actions against every concrete player branch.
- Enforce robust coverage and regret.
- Restrict only expensive future planning to the relevant 3x3 subset.

**Exit:** the resident planner produces bounded action-aware rankings within the
runtime and memory gates.

## Phase 8 — Commitments and calibration

- Implement genuine specialized-read commitments.
- Resolve AI win, player win, neutral and surprise outcomes.
- Add the small recent-battle case window without an automatic repeat penalty.
- Add 32 persistent abstract player-behaviour cases and the versioned aggregate
  header only after save-layout, serialization and migration review.

## Phase 9 — Optimization and tuning

- Measure resident code/data/BSS, battle-heap state, stack, aligned save growth
  and each timing component.
- Require the 500-byte resident reserve, 768-byte battle-owned-state maximum,
  minimal separately reported overlay BSS, sub-1 KiB stack, <=272-byte logical
  persistent store and two-frame ordinary decision target.
- Tune relationships with deterministic scenarios before adding rare special
  cases.

# 18. Verification and acceptance

No build, emulator run or Pokémon test is implied by editing this document.
When separately authorized, verification must cover the following.

## 18.1 Integration

- Non-expert trainers and unsupported formats use original HGSS behavior.
- Bag use is disabled for both sides; expert singles neither generates,
  predicts nor selects ITEM.
- Command and move hooks return the same cached decision.
- Post-KO selection cannot trigger during initial setup or reuse stale state.
- Repeated battles clear trainer-specific state.

## 18.2 Command blindness

- Changing only the player's preselected command/move/switch/target fields does
  not change an AI decision with identical sanitized state and RNG.
- Changing exact loadout or calculated stats may change the decision.
- No strategic helper receives selected-command fields or an unrestricted live
  pointer capable of bypassing the sanitizer.

## 18.3 Tactical and strategic preferences

- guaranteed KO > pointless setup;
- damaging immunity < useful safe coverage;
- safe hazard-aware switch > lethal switch;
- recovery that changes survival > low-value chip;
- setup that reduces total winning turns and survives > repeated weak attacks;
- setup that dies before payoff < immediate survival/attack;
- self boost generally outvalues an equivalent target drop when the opponent
  can freely switch;
- target drop retains full value against the last/trapped opponent;
- useful switch-forcing tempo/hazard value is recognized;
- semantics gaps are classified in the tracked coverage report and remain
  action-local;
- conditional secondaries have zero value when they cannot apply and retain
  only their mechanically valid current/future value otherwise;
- legal random-selection moves are low-positive desperation options, while
  currently unusable instances are invalid.

## 18.4 Prediction

- Every legal player move and switch remains represented in immediate
  coverage/regret evaluation; only expensive future simulation is narrowed.
- The trainer shortlist contains counters needed by high-probability and
  dangerous credible branches rather than only unconditional top scores.
- Coverage target and regret alter eligibility.
- Robust choices create no prediction commitment.
- Failed specialized predictions are resolved and influence learned action
  probabilities without an automatic repeat ban.
- Counterfactual-unavoidable outcomes remain neutral.
- Recent-battle cases clear between unrelated trainers.
- The 32 persistent cases contain no trainer, Pokémon, move, item, ability,
  slot or matchup identity.

## 18.5 Capacity, placement and runtime

- Overlay 130 retains at least 500 bytes by final linked output/highest
  allocated section end.
- Battle-owned state/workspace is at most 768 bytes and is cleared with the
  owning battle; overlay BSS is reported separately.
- Worst nested AI stack is below 1 KiB.
- No transient overlay artifact, ID, linker image, loader or evaluator placement
  exists unless separately and explicitly approved after a measured resident
  fit failure.
- One cached decision evaluates at most once for command/move consumption.
- Ordinary total decision time is within two video frames, with each timing
  component reported separately.

# 19. Risks and mitigations

| Risk | Mitigation |
|---|---|
| AI reads the selected command indirectly | Central sanitizer, explicit denylist, helper audit and command-blindness tests |
| Overlay-130 exhaustion | Compact shared semantics, remove inference/duplication, enforce the 500-byte reserve, stop with measurements and ask before proposing transient placement |
| Transient overlay is introduced casually | Explicit approval gate; general implementation/build authority is insufficient |
| Resident snapshot/evaluation dominates runtime | Exact cached stats, compact descriptors and per-decision memoization |
| Search explodes | Stream a bounded <=9x9 cheap scan and <=4x9 full first-turn evaluation; use analytic plans and a maximum 3x3 expensive future subset |
| Unsafe prediction chooses an immunity | Enforced coverage/regret, catastrophic credible branch and safe-action eligibility |
| AI becomes predictably repetitive | Persistent abstract cases, current match position and a calculated mixed strategy among non-dominated actions when counter-reading evidence justifies it |
| Strategic evaluator lacks a move case | Track it in the nonblocking Markdown backlog and apply only the documented action-local conservative behavior |
| Stat drops are overvalued | Weight by opponent stay probability, switch tempo and last/trapped status |
| Self boosts are overvalued | Weight by trainer survival and probability it remains active |
| Saved prediction memory captures identities | Persist 32 strategic-class cases only; reject trainer/species/move/item/ability/slot keys and version the store |
| Doubles reaches singles logic | Gate format before state access and delegate to original HGSS |
| Debug payload consumes release space | Compile diagnostics out or omit them |

# 20. Initial tuning parameters

These build-time values live in `include/ai_config.h` under `AI_CFG_*` names.
Values marked Tune are deliberately unresolved until deterministic scenarios
and measured runtime exist. Prefer relational tests over memorizing one score.

| Parameter | Initial value/target | Meaning |
|---|---:|---|
| `AI_CFG_PROBABILITY_SCALE` | 255 | Q8-style normalized probability total |
| `AI_CFG_HP_BAR_UTILITY` | 256 | Utility represented by one complete HP bar |
| `AI_CFG_ROBUST_COVERAGE_TARGET` | ~230/255 | Required covered probability mass when achievable |
| `AI_CFG_COVERAGE_MAX_REGRET` | Tune | Maximum branch loss for “covered” |
| `AI_CFG_COVERAGE_MIN_OUTCOME` | Tune | Minimum acceptable branch utility |
| `AI_CFG_MAX_FUTURE_PLAYER_CANDIDATES` | 4 | Branches eligible for deep selection; immediate evaluation retains every legal move/switch |
| `AI_CFG_MAX_PLAYER_MOVE_SWITCH_ACTIONS` | 9 | Singles bound: four moves plus five destinations |
| `AI_CFG_MAX_SEARCH_AI_ACTIONS` | 4 | Trainer actions receiving full first-turn evaluation |
| `AI_CFG_MAX_FULL_SWITCH_CANDIDATES` | 3 | Role-distinct reserves receiving contextual evaluation |
| `AI_CFG_MAX_SEARCHED_SWITCH_ACTIONS` | 3 | Role-distinct switches admitted to final scoring |
| `AI_CFG_MAX_FUTURE_AI_ACTIONS` | 3 | Expensive future subset |
| `AI_CFG_MAX_FUTURE_PLAYER_ACTIONS` | 3 | Expensive future subset |
| `AI_CFG_FORCED_FUTURE_PERCENT` | 95 initial | Retained value for mechanically forced continuations such as a successful Rollout lock |
| `AI_CFG_SPECULATIVE_FUTURE_PERCENT` | 56 initial | Retained value when the opponent receives another unrestricted decision |
| `AI_CFG_PLAN_TURN_CAP` | 6 | Saturation cap for analytic turns-to-KO comparison |
| `AI_CFG_READ_COMMITMENT_GAP` | 10 initial | Minimum stay-branch sacrifice before a selected switch read creates a prediction commitment |
| `AI_CFG_COMFORTABLE_RISK_WEIGHT` | Tune/high | Protect an established winning position |
| `AI_CFG_EVEN_RISK_WEIGHT` | Tune | Permit calculated mixups |
| `AI_CFG_DESPERATE_RISK_WEIGHT` | Tune/low | Permit variance when robust lines lose |
| `AI_CFG_HARD_READ_MIN_PAYOFF` | Tune | Required gain before sacrificing meaningful coverage |
| `AI_CFG_READ_BALANCE_CLAMP` | -8..+8 | Prevent aggregate calibration runaway |
| `AI_CFG_PERSISTENT_PREDICTION_CASES` | 32 | Abstract cross-battle player-behaviour cases |
| `AI_CFG_RECENT_BATTLE_CASES` | <=8 | Volatile immediate-adaptation window |
| `AI_CFG_RANDOM_ACTION_BASE_VALUE` | Low positive | Keeps legal random-selection actions available mainly for desperate positions |
| `AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT` | 40% initial | Future value retained for target-bound effects when a legal switch can clear them; immediate effect and forced-switch tempo are evaluated separately |
| `AI_CFG_UNKNOWN_ACTION_BASE_VALUE` | Low positive | Individual conservative value for a documented evaluator gap; never global decision randomness |
| `AI_CFG_TEAM_ASSET_WEIGHT` | 48 initial | Base value of retaining a healthy trainer Pokémon |
| `AI_CFG_UNIQUE_COVERAGE_WEIGHT` | 24 initial | Marginal value for matchups no surviving teammate covers as well |
| `AI_CFG_UNIQUE_DEFENSE_WEIGHT` | 16 initial | Marginal value for irreplaceable defensive responses |

Compile-time checks enforce action/deep-search relationships, valid Q8 ranges,
case-count/storage limits and the resident scratch/stack gates. Tuning must not
change the no-Bag action space or enable a transient overlay.

# Appendix A. Proposed narrow API

```c
/* Lifecycle and integration. */
void AI_ExpertSinglesBattleInit(struct BattleSystem *bsys);
void AI_ExpertSinglesBattleEnd(struct BattleSystem *bsys);
u8 FairTrainerAI_PickCommand(struct BattleSystem *bsys, u8 battlerId);
u8 FairTrainerAI_PickMove(struct BattleSystem *bsys, u8 battlerId);
u8 FairTrainerAI_PickReplacement(struct BattleSystem *bsys, u8 battlerId);

/* Sole exact-state sanitizer. */
void AI_BuildExactSnapshot(const struct BattleSystem *bsys,
                           u8 aiBattler,
                           AIExactSnapshot *out);

/* Handwritten interface over the generated semantics data. */
BOOL AI_GetMoveSemantics(u16 moveId,
                         u16 moveEffect,
                         AIMoveSemantics *out);
void AI_ApplyMoveSemantics(const AIMoveSemantics *semantics,
                           const AIMoveContext *context,
                           AISimState *state);

/* Resident bounded decision pipeline. */
BOOL AI_EvaluateExpertSingles(const AIExactSnapshot *snapshot,
                              AIPlannerResult *result);
```

Exact signatures must follow the source integration seams. The battle engine
should not depend on internal evaluator structs.

# Appendix B. Example decision

The player has a Ground-type active Pokémon and two legal reserves. The trainer
has Thundershock, a neutral damaging move, a setup move and legal switches.

1. The exact snapshot contains all four player moves, both reserve loadouts,
   exact stats, abilities and items. It does not contain the player's selected
   command.
2. Player action generation assigns a probability to every exact legal attack
   and both switch destinations. All participate in immediate coverage; only
   the most relevant branches are eligible for deep future simulation.
3. A cheap complete response scan ensures that counters to both likely and
   dangerous credible branches can enter the trainer shortlist. Trainer switch
   screening admits the two reserves with the best entry-survival,
   pressure/threat and remaining-HP positions.
4. Thundershock receives a catastrophic outcome when the current Ground target
   stays and a favorable outcome only for switch destinations it can hit.
5. If safe neutral coverage reaches the robust target, Thundershock is not
   eligible for the robust policy or a calculated mixed strategy.
6. If no robust action exists and the switch probability/payoff genuinely
   justifies Thundershock, selecting it creates a prediction commitment.
7. The next decision resolves whether the player actually switched and whether
   another feasible action would have covered the result. The outcome updates
   the recent and persistent abstract cases, which alter later probabilities
   and mixup breadth without imposing an identical-repeat penalty.

# Appendix C. References and source evidence

- Commit `ad0d002a1f4dc2eb20782391bdfb39ec58ebee17`, including its removed
  `src/battle/trainer_ai.c`: legacy expert-AI behavior used as Section 3
  regression evidence, not as the clean implementation baseline.
- `documentation/TRAINER_AI_LEGACY_ROLLBACK_CHANGELIST.md`: authoritative
  source rollback boundary, including preserved engine/content changes.
- The removed `include/trainer_ai.h` plus the introducing commit's `hooks` and
  `src/battle/battle_start.c` changes: verified legacy integration addresses
  and lifecycle evidence to reconstruct deliberately in Phase 1.
- `src/overlay.c`, `src/battle/battle_calc_damage.c` and
  `src/individual/linker/CalcBaseDamage.ld`: existing fixed-region individual
  overlay pattern.
- `documentation/CODE_BUDGET.md`: historical measurements only; fresh Phase 0
  output supersedes them.
- BluRosie/hg-engine battle AI and battle-structure sources remain mechanics
  references, but local Heartless Gold integration and measurements take
  precedence.

> **Next engineering artifact:** Produce a Phase-0 source/budget audit that
> confirms the clean baseline, replaces stale generated size evidence and
> lists each Section-3 regression requirement before implementing v0.6.
