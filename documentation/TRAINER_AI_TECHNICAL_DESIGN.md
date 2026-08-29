# Heartless Gold Expert Trainer AI Technical Design

An exact-information, action-blind, prediction-aware singles trainer AI for
Pokémon HeartGold hg-engine.

| Field | Value |
|---|---|
| Status | Restart-ready implementation design (v0.5) |
| Scope | Expert singles trainer battles only |
| Production profile | `F_TRAINER_EXPERT_AI` |
| Primary goals | Strong tactics, smart switching, multi-turn planning, player-action prediction, bounded runtime, measurable size |
| Explicit non-goals | Doubles AI, full battle-script simulation, deep game-tree search, runtime machine learning, reading the player's selected command |
| Source baseline | Current post-legacy-rollback source worktree: original HGSS AI plus preserved Heartless Gold engine/content fixes; generated size output still requires a fresh authorized build |

> **Document status:** Proposed types, functions and parameters are design
> guidance. Section 3 inventories the removed committed legacy AI as regression
> and integration evidence. It is not the current implementation. The
> exact source rollback is specified in
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

Three states must remain distinct in implementation notes and size reports:

1. **Removed committed legacy AI:** the fair-information implementation
   introduced by `ad0d002a1f4dc2eb20782391bdfb39ec58ebee17`.
2. **Current clean rollback baseline:** original HGSS AI selected through the
   preserved trainer flags, plus unrelated Heartless Gold fixes and content.
3. **Discarded attempt:** the v0.4 overlay/lookahead scaffold that produced the
   battle-entry and per-turn performance investigation.
4. **v0.5 target:** the design in this document.

No generated file, stale object or ROM output is evidence that a reverted
source tree implements v0.5. Phase 0 establishes a fresh source and size
baseline after both AI rollbacks.

## 0.1 Major v0.5 changes

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
- Retain the transient planner architecture. Overlay 130 holds the smallest
  safe resident shell; the complex evaluator belongs in the transient overlay.
  Exact-information simplification should increase resident headroom, not
  justify consuming the 1 KiB reserve.
- Do not add an “obvious decision” fast path in the first v0.5 implementation.
  One cached non-forced expert decision may make one transient call.
- Make overlay-load time, resident request-building time and transient search
  time separate performance measurements. The discarded attempt did not do
  this and therefore cannot establish which component caused its pause.
- Add analytic multi-turn plan evaluation for setup, stat changes, recovery and
  support. A six-turn payoff must not require a six-ply search tree.
- Screen every legal trainer reserve cheaply, fully evaluate the best three
  role-distinct candidates, and admit at most two switches to the final four
  searched AI actions.
- Make coverage and maximum regret enforceable selection constraints. A
  damaging move into an immunity is not a safe action merely because it is
  strong against one predicted switch.
- Treat an evaluator-unknown action individually. One move that lacks a
  strategic scoring case must never randomize the entire decision.
- Implement the prediction commitment, resolution, trainer-local memory and
  global calibration described here. They existed in v0.4 documentation but
  not in either the committed legacy AI or the discarded implementation.

# 1. Executive summary

The expert AI is a bounded decision system for singles trainer battles. It
knows both parties' complete battle-relevant loadouts but remains blind to the
player's current command. It compares moves and switches on a compatible
utility scale, predicts several plausible player actions, selects robust lines
when available and records an explicit prediction commitment only when it
chooses a specialized read.

The design has five layers:

1. A resident integration shell validates the battle format/profile, snapshots
   exact state, generates legal actions, caches decisions and provides a legal
   tactical fallback.
2. A compact tactical model calculates damage, KO timing, speed, survival,
   hazards, status and other immediate effects from exact sanitized values.
3. A plan evaluator compares multi-turn attack, setup, recovery, support and
   switch plans without expanding a deep game tree.
4. A player-action model assigns probabilities to exact legal moves and switch
   destinations, retaining four primary branches plus compact tail identities.
5. A transient zero-import overlay performs pairwise coverage, bounded future
   evaluation, prediction commitment generation and final ranking.

The AI never returns ITEM. Configured trainer Bag items may remain in trainer
data. Doubles and unsupported formats delegate to the original HGSS AI.

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
- Remember genuine read outcomes and avoid blindly repeating failed reads.
- Preserve at least 1,024 linked bytes in overlay 130.
- Keep runtime bounded and ordinary decision latency below two video frames.
- Measure strength, size and runtime at every implementation phase.

## 2.2 Non-goals

- Doubles, multi, tag or other multi-battler strategy.
- Full battle-script execution in the evaluator.
- Exact simulation of every unusual move effect on the first pass.
- Deep minimax over every future player response.
- Runtime learning of move mechanics or trainer data.
- Reading a player's current selected move, switch, target or command.
- Retaining player-specific identities across unrelated trainers.
- A large release diagnostic or explanation framework.

## 2.3 Non-negotiable invariants

| Invariant | Requirement |
|---|---|
| Command blindness | The AI may read exact loadout/state but never current player command, selected move slot, switch destination or target. |
| No indirect command leak | No helper used before selection may infer the current player command from battle-controller fields. |
| Exact snapshot boundary | Transient code receives pointer-free value data, never live battle, battler or party pointers. |
| Legal action only | Every returned move/switch index is validated resident-side before use. |
| Unsupported-format fallback | Doubles and unsupported formats delegate before singles-only state is accessed. |
| One production profile | Only `F_TRAINER_EXPERT_AI` selects the advanced path. |
| No trainer Bag action | Expert singles never generates or selects ITEM. |
| Cached decision | Command and move hooks consume the same decision; stochastic work runs once. |
| Coverage is enforceable | Coverage and regret thresholds affect eligibility, not merely a small score bonus. |
| Per-action evaluator fallback | An unscored action cannot randomize other understood actions. |
| Prediction honesty | Robust actions create no read record; unavoidable losses create no false player-win record. |
| Scoped memory | Trainer-specific cases clear between trainers; only two abstract calibration bytes may persist. |
| Resident headroom | Overlay 130 retains at least 1,024 linked bytes. |
| Bounded state | Resident AI BSS/workspace is at most 768 bytes; worst nested AI stack is below 1 KiB. |
| Overlay safety | Transient code cannot import resident functions, nest another individual-overlay load, yield or retain transient addresses. |
| Determinism | Search consumes no production battle RNG; final safe near-best sampling is the only decision RNG use. |

# 3. Removed legacy AI inventory and rollback boundary

This section documents the removed fair-information AI from commit
`ad0d002a1f4dc2eb20782391bdfb39ec58ebee17`: its evaluator, interface, hooks
and battle-start reset. Its covered mechanics and verified seams remain
evidence for v0.5 requirements, not code in the current baseline.

## 3.1 Verified integration evidence to retain in the design

- The legacy runtime uses `IMPLEMENT_FAIR_TRAINER_AI` and
  `TrainerAI_UsesStrategicLayer()`. Both are removed with that runtime.
- The overlay-12 command and move call sites have verified addresses:

  ```text
  0012 FairTrainerAI_PickCommand 0225E10E bl
  0012 FairTrainerAI_PickMove    0225E43E bl
  ```

- The legacy hook lines are removed for the clean rollback baseline, then v0.5
  reintroduces guarded wrappers at the verified addresses with instruction
  assertions.
- The original HGSS picker addresses and fallback behavior are known. Linker
  exports are removed if unused during rollback and re-added only when v0.5
  needs them.
- `F_TRAINER_EXPERT_AI` is preserved. It predates the legacy implementation,
  expands to the three original HGSS expert modules and remains the production
  content profile.
- The v0.5 command and move selections must share a cache keyed by turn and
  active party slot.
  This prevents two independent decisions for one turn and protects against a
  replacement inheriting a fainted Pokémon's decision.
- The legacy ability/held-item observation calls are removed. Exact-information
  v0.5 refreshes its value snapshot through one central sanitizer instead of
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
- quantized player HP rather than exact player HP, which v0.5 intentionally
  replaces with exact sanitized state.

These algorithms are removed with the legacy runtime. They are not
automatically correct, but their covered mechanics form a regression inventory
for the replacement.

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
  v0.5 singles/unsupported-format gate before accessing the custom evaluator.
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

Configured trainer Bag items remain. The temporary original-HGSS baseline may
use them; the v0.5 expert singles wrapper is responsible for never generating
or selecting ITEM.

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

Recorded evidence, not a post-legacy-rollback measurement:

| Source state | Overlay 130 used | Free | Notes |
|---|---:|---:|---|
| Committed legacy-AI measurement | 77,772 B | 4,148 B | Recorded before the discarded attempt; not the clean rollback baseline |
| Estimated committed legacy AI feature | 8,172 B | — | Prior controlled delta; historical only |
| Estimated clean rollback baseline | ~69,600 B | — | Consistent with the dated 69,496-byte disabled build; replace with a fresh post-rollback build |
| Discarded attempt artifact | 80,120 B | 1,800 B | Latest generated artifact inspected during the performance investigation |

The discarded attempt also produced a 384-byte AI BSS object and a 676-byte
transient overlay. These numbers describe an incomplete scaffold and are not
v0.5 targets.

The hard resident gate is:

```text
output_battle.bin <= 81,920 - 1,024 = 80,896 bytes
```

The 1 KiB is linker headroom for future resident fixes and integration—not
battle heap. v0.5 should use exact-information simplification to increase that
headroom. Do not move transient planner code resident merely to remove the
transient overlay.

## 4.2 Transient planner region

```text
Start:    0x023C0400
Length:   0x00003C00
End:      0x023C4000
Capacity: 15,360 bytes
```

The transient planner target is 6–9 KiB and must remain below 15,360 bytes.
This is a separate budget from overlay 130, BSS, stack, battle heap, save data
and total ROM size.

## 4.3 Other hard budgets

| Memory class | Gate |
|---|---:|
| Resident AI state/workspace | <=768 B of overlay-130 BSS |
| Worst nested AI stack | <1 KiB |
| Persistent calibration | Two logical bytes; measure aligned save growth |
| Release diagnostics | No strings or retained score-component arrays |

# 5. Architecture and ownership

## 5.1 Resident overlay 130

Resident code owns:

- profile and singles-format gating;
- battle lifecycle and state clearing;
- exact loadout/state sanitization;
- legal move/switch enumeration and compact move/state descriptor generation;
- decision cache and final engine output;
- a compact, competent tactical fallback;
- construction and validation of pointer-free request/result buffers;
- transient overlay collision preflight and lifetime; and
- resolving prior prediction commitments after the actual action is known.

Resident code should not own the full prediction/search implementation merely
because it is callable every turn.

## 5.2 Transient planner

Transient code owns pure calculations for:

- player action probabilities;
- player switch-destination ranking;
- trainer switch shortlisting and role comparison;
- analytic setup/support/recovery plans;
- pairwise action/branch outcomes;
- bounded future-state evaluation;
- coverage, regret and robust eligibility;
- prediction commitment generation; and
- final action ranking.

The discarded attempt inverted this boundary: resident code performed most
damage, threat, switch and pair construction, while the 676-byte overlay mostly
ranked precomputed scores. v0.5 must not repeat that architecture.

## 5.3 Runtime flow

```text
battle state
    -> resident exact-state sanitizer
    -> legal AI/player actions plus compact Pokémon/move descriptors
    -> pointer-free planner request
    -> one synchronous transient call
        -> player distribution and trainer switch screening
        -> tactical caches, plans, pairwise coverage and ranking
    -> resident result validation
    -> safe near-best final sample
    -> cached command/move output
    -> later commitment resolution
```

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
    u32 condition;
    u32 condition2;
} AIMonView;
```

This is guidance, not a frozen ABI. Order fields deliberately and enforce a
compile-time size check. A request should contain only the active and shortlisted
views needed for that decision, not two complete six-member parties when compact
derived descriptors are sufficient.

The transient interface contains no pointers except the resident caller's two
entrypoint arguments to the top-level request and result objects.

Move IDs alone are insufficient for a zero-import evaluator. The resident
sanitizer must also provide compact immutable move descriptors containing the
fields the planner needs, such as power, type, split, accuracy, priority,
targeting, effect class and supported effect parameters. The transient planner
must not call the live move table or a resident move-data helper.

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

Generate every legal move and voluntary switch. ITEM is intentionally absent.
Forced Struggle and forced replacement use dedicated legal paths.

## 7.2 Evaluator coverage

“Evaluator-unknown” means the battle engine implements the action but v0.5 has
no strategic scoring model for its full effect. It does not mean the move is
unimplemented or that trainers are allowed to learn invalid content.

Rules:

- A known damaging component is still scored when a secondary effect is not.
- An unknown strategic component receives a named conservative adjustment.
- An evaluator-unknown action remains an individual candidate.
- It may use a narrow original-AI/tactical fallback score if that interface is
  deterministic and action-local.
- It must never set a global flag that makes every legal action uniformly
  random.
- Maintain a static report of trainer moves whose strategic effect lacks an AI
  model so coverage work is explicit.

# 8. Tactical evaluation and caching

## 8.1 Tactical primitives

The core must represent:

- expected and conservative damage;
- type immunity/resistance/weakness and STAB;
- accuracy, priority and effective speed order;
- current HP and KO thresholds;
- fixed-damage and common variable-power moves;
- status legality and immediate value;
- recovery and survival;
- entry hazards and switch-in survival;
- supported items and abilities; and
- current stages, weather, terrain/field effects and screens where implemented.

The production battle RNG is never consumed by evaluation. Use expected or
bounded conservative values.

## 8.2 Per-decision cache

Exact player data removes belief expansion but not repeated arithmetic. The
transient planner uses small fixed per-invocation caches keyed by sanitized
view/action identity:

- damage for retained attacker/target/move combinations;
- incoming threat for retained player branches;
- effective speed/priority relationships;
- hazard damage for shortlisted switches; and
- post-stage best damage used by plan summaries.

Use compact transient stack/static workspace or tightly bounded resident
request/result scratch where lifetime requires it. Do not add a large
permanent matrix to overlay-130 BSS.

## 8.3 Immunity and tactical-safety rules

- A damaging move that deals zero to the current target records a catastrophic
  stay-branch outcome, not a neutral zero.
- Such a move may remain a specialized prediction candidate only when the
  switch branch is sufficiently credible and no safe alternative satisfies the
  configured coverage/regret constraints.
- If a safe legal action covers the current target and the likely switch, an
  immunity gamble cannot enter safe near-best sampling.
- The same failed specialized prediction receives a short-term repeat penalty
  unless new state materially changes its probability or payoff.

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

# 10. Trainer switching and post-KO selection

## 10.1 Cheap screen of all reserves

The transient planner scores every legal reserve using inexpensive exact-state
descriptors supplied by resident code:

- hazard damage and survival after entry;
- expected incoming damage, resistance or immunity;
- current HP and status;
- speed and revenge-KO pressure;
- best direct attack;
- recovery, setup, status, screens or other support value; and
- recent-switch/zero-progress penalties.

## 10.2 Fully evaluate three role-distinct candidates

Retain up to three distinct candidates:

1. safest defensive pivot;
2. strongest immediate attacker/revenge killer; and
3. best setup/support option.

Deduplicate when one Pokémon fills multiple roles. Fully evaluate those three
against the retained player branches. At most two switch actions enter the
final four searched AI actions, leaving room for at least two move actions.

Post-KO replacement uses the same role and plan model but has no “switch costs
this turn” penalty. It enters through the guarded seam in Section 3.4 and has
its own cache mode/generation.

# 11. Player action prediction

The AI knows exactly which actions the player possesses but not which action
the player chose. Possession inference is removed; choice prediction remains.

## 11.1 Candidate generation

Generate:

- every legal player move;
- every legal player switch destination; and
- forced actions where applicable.

Cheaply score candidates from the player's perspective using exact state.
Retain four primary branches and up to twelve compact total identities. Low
probability but legal actions remain identifiable tail actions; “tail” no
longer means an unknown move.

## 11.2 Initial probability model

Weights may consider:

- KO and damage value;
- survival and speed;
- setup/recovery/status value;
- switch matchup, hazard cost and remaining HP;
- whether the active Pokémon is threatened with a KO;
- trainer-local cases; and
- abstract global read/surprise calibration.

Do not pick the predicted switch merely by highest HP. Rank every legal player
reserve by its ability to survive the trainer's likely attacks, threaten back,
support another switch or create setup pressure.

Use smoothing so a single observation cannot create certainty.

# 12. Pairwise evaluation, coverage and selection

## 12.1 Bounded search

- Fast-score all legal AI actions.
- Retain at most four AI actions, including the best move and a switch when
  each class has a credible candidate.
- Retain four primary player branches.
- Calculate direct pairwise utility for all retained pairs.
- Apply the more expensive future-plan evaluation to at most a relevant 3x3
  subset.
- Unsupported simulated mechanics retain their direct pairwise value.
- Stream results into per-action summaries; do not retain a production matrix
  after the decision.

## 12.2 Coverage semantics

For each AI action and player branch, calculate:

- direct/future utility;
- regret relative to the best available response to that branch; and
- whether the outcome meets both minimum utility and maximum regret.

Covered mass is the sum of probabilities for covered branches. Selection:

1. If one or more actions meet `robustCoverageTarget`, only those actions are
   robust-eligible.
2. Choose among safe near-best robust actions unless configured payoff clearly
   justifies a specialized prediction.
3. If no action meets the coverage target, compare weighted value, worst
   credible outcome and regret explicitly.
4. An immunity or other catastrophic credible branch cannot be hidden by a
   small coverage bonus.
5. Random sampling occurs only among actions that already satisfy the same
   tactical-safety class.

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
    u16 coveredPrimaryMask;
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
| Actual legal action was retained only in tail and was counterable | Counterable surprise |
| Tail action was not counterable | Neutral surprise |

Exact loadout knowledge removes the old “newly revealed unknown move” class.

## 13.3 Trainer-local cases

Retain eight compact situation/action/outcome cases for the current trainer.
They may influence repeated choices in similar situations and are cleared
before an unrelated trainer battle. Target eight bytes per case and enforce the
compiled size.

## 13.4 Cross-battle calibration

Only two abstract clamped bytes may persist:

```c
typedef struct {
    s8 readBalance;
    u8 surpriseTendency;
} AIPredictionCalibration;
```

They may adjust probability sharpness, robust-versus-specialized tolerance,
tail mass and safe mixup breadth. They may not encode species, move, item,
ability, slot, trainer or matchup identity.

A failed specialized read applies both the normal commitment outcome and a
short-term trainer-local penalty against immediately repeating the identical
read. New evidence or a materially changed state may clear that penalty.

# 14. Transient overlay contract

The transient overlay is code and local static data loaded into a fixed reusable
EWRAM region. It is not a temporary heap allocation.

## 14.1 Link/build enforcement

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

## 14.2 Resident wrapper

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

## 14.3 Critical prohibitions

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
- trainer switch screening;
- request construction;
- overlay load;
- transient plan/search evaluation;
- unload/result validation; and
- total decision time.

The discarded attempt's visible pause cannot be attributed to overlay loading
alone: its overlay was only 676 bytes while resident code repeatedly performed
damage, personal-data, threat, reserve and pair calculations. Conversely, the
load is synchronous fixed cost and must still be measured rather than assumed
cheap.

## 15.2 Runtime gates

- Ordinary decisions target at most two video frames total.
- Forced actions do not load the planner.
- Command and move hooks load at most once through the decision cache.
- No initial “obvious decision” fast path is required by v0.5.
- If runtime exceeds the target, optimize duplicated primitives and reduce the
  expensive 3x3 subset before removing player prediction breadth.
- If measured overlay load alone prevents the target, stop and reconsider the
  placement/lifetime architecture; do not consume the overlay-130 reserve as
  an unmeasured workaround.

# 16. Decision pipeline

```text
AI_ThinkExpertSingles
    reject unsupported format/profile -> original HGSS AI
    resolve previous prediction commitment if actual action is available
    reuse valid command/move cache
    refresh exact sanitized state
    enumerate legal AI/player moves and switches
    build compact Pokémon, move and field descriptors
    build pointer-free request
    synchronously run transient planner once
        build compact tactical caches
        cheaply screen all trainer reserves
        retain up to three reserve roles
        fast-score all AI actions
        retain four AI actions (at most two switches)
        generate the player action distribution
        evaluate plans, coverage and ranking
    validate result resident-side
    select one safe near-best action with battle RNG
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
  largest symbols and exact 1 KiB headroom.
- Verify the legacy hooks and runtime references are absent. Retain their
  verified addresses as documentation for Phase 1.
- Verify overlay ID availability and the fixed transient region.
- Create a regression inventory from Section 3.

**Exit:** baseline behavior and all memory budgets are measured rather than
estimated.

## Phase 1 — Integration shell and command-blind boundary

- Add the singles/unsupported-format gate before custom state access.
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

**Resident BSS/code and stack checkpoints required.**

## Phase 3 — Transient shell

- Add the explicit ID/linker region, entry assertion and zero-import build.
- Add static poison/undefined-symbol checks.
- Implement collision, load, invalid-result and unload-once handling.
- Measure load/unload independently with a trivial request.

**Exit:** safe round trip and fallback are proven before planner complexity.

## Phase 4 — Tactical core and evaluator coverage

- Implement exact damage, KO, immunity, speed, priority, accuracy, recovery and
  status legality.
- Define compact move/field descriptors and implement transient per-decision
  primitive caching without resident mechanics imports.
- Add individual conservative fallback for evaluator-unknown strategic
  effects and generate the coverage report.
- Make immunity outcomes tactically catastrophic where appropriate.

**Exit:** obvious KOs, immunities and legal tactical choices order correctly.

## Phase 5 — Switching

- Add hazard/survival screening for every reserve.
- Retain three role-distinct candidates and fully evaluate them.
- Admit at most two switches to final search.
- Add voluntary-switch and post-KO regression scenarios.

## Phase 6 — Multi-turn plans

- Implement analytic attack/setup/drop/recovery/support plan summaries.
- Account for turns to KO, incoming actions, survival and effect persistence.
- Model player and trainer switching when valuing self boosts and target drops.

**Exit:** relational scenarios such as Leer plus four Tackles versus six
Tackles produce positionally sensible choices.

## Phase 7 — Prediction, coverage and bounded planner

- Generate probabilities over exact legal player actions.
- Retain four primary branches plus compact tail identities.
- Integrate the transient tactical, switch and plan modules into pairwise
  branch evaluation and coverage ranking.
- Enforce robust coverage and regret.
- Evaluate 4x4 direct pairs and at most a 3x3 future subset.

**Exit:** the overlay performs meaningful planner work rather than only ranking
resident-precomputed pairs.

## Phase 8 — Commitments and calibration

- Implement genuine specialized-read commitments.
- Resolve AI win, player win, neutral and surprise outcomes.
- Add eight trainer-local cases and failed-read repeat handling.
- Add two abstract persistent calibration bytes only after save-layout review.

## Phase 9 — Optimization and tuning

- Measure resident/transient bytes, state, stack, save growth and each timing
  component.
- Require the 1 KiB resident reserve, 15,360-byte transient maximum, 768-byte
  state maximum, sub-1 KiB stack and two-frame ordinary decision target.
- Tune relationships with deterministic scenarios before adding rare special
  cases.

# 18. Verification and acceptance

No build, emulator run or Pokémon test is implied by editing this document.
When separately authorized, verification must cover the following.

## 18.1 Integration

- Non-expert trainers and unsupported formats use original HGSS behavior.
- Expert singles never selects ITEM.
- Command and move hooks return the same cached decision.
- Post-KO selection cannot trigger during initial setup or reuse stale state.
- Repeated battles clear trainer-specific state.

## 18.2 Command blindness

- Changing only the player's preselected command/move/switch/target fields does
  not change an AI decision with identical sanitized state and RNG.
- Changing exact loadout or calculated stats may change the decision.
- No transient request contains live pointers or selected-command fields.

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
- evaluator-unknown action does not randomize understood actions.

## 18.4 Prediction

- Coverage target and regret alter eligibility.
- Robust choices create no prediction commitment.
- Failed specialized predictions are resolved and are not blindly repeated.
- Counterfactual-unavoidable outcomes remain neutral.
- Trainer-local cases do not leak to unrelated trainers.
- Global calibration contains no identifying battle data.

## 18.5 Capacity, overlay and runtime

- Overlay 130 retains at least 1,024 bytes by final linked output/highest
  allocated section end.
- Resident state/workspace is at most 768 bytes.
- Worst nested AI stack is below 1 KiB.
- Transient output is below 15,360 bytes and its entrypoint assertion passes.
- Undefined-symbol and forbidden-call checks pass.
- Occupied region, rejected load and invalid result preserve the existing owner
  and select resident fallback.
- One cached decision loads at most once and unloads exactly once after success.
- Ordinary total decision time is within two video frames, with each timing
  component reported separately.

# 19. Risks and mitigations

| Risk | Mitigation |
|---|---|
| AI reads the selected command indirectly | Central sanitizer, explicit denylist, helper audit and command-blindness tests |
| Overlay-130 exhaustion | Replace old AI, remove belief/inference code in favor of exact snapshots, keep planner transient, enforce 1 KiB reserve |
| Transient nested load overwrites itself | Zero-import link, forbidden API poison, undefined-symbol check and source review |
| Transient region is occupied | Preflight and resident fallback; never auto-unload another owner |
| Overlay load causes visible pause | Measure load separately; ensure complex work is transient; optimize before changing architecture |
| Resident request building dominates runtime | Exact cached stats, compact descriptors and per-decision memoization |
| Search explodes | Four player branches/four AI actions, analytic plans, maximum 3x3 future subset |
| Unsafe prediction chooses an immunity | Enforced coverage/regret, catastrophic credible branch and safe-action eligibility |
| Failed predictions repeat | Commitment resolution plus short-term identical-read penalty |
| Strategic evaluator lacks a move case | Per-action conservative fallback and coverage report, never global randomization |
| Stat drops are overvalued | Weight by opponent stay probability, switch tempo and last/trapped status |
| Self boosts are overvalued | Weight by trainer survival and probability it remains active |
| Save calibration shares trainer identity | Persist only two clamped abstract bytes |
| Doubles reaches singles logic | Gate format before state access and delegate to original HGSS |
| Debug payload consumes release space | Compile diagnostics out or omit them |

# 20. Initial tuning parameters

Values are placeholders. Prefer relational tests over exact expected totals.

| Parameter | Initial value/target | Meaning |
|---|---:|---|
| `robustCoverageTarget` | 90% | Required covered primary probability mass when achievable |
| `coverageMaxRegret` | Tune | Maximum branch loss for “covered” |
| `coverageMinOutcome` | Tune | Minimum acceptable branch utility |
| `maxPlayerPrimary` | 4 | Expensive player branches |
| `maxPlayerIdentities` | 12 | Primary plus compact tail identities |
| `maxSearchAiActions` | 4 | Directly compared AI actions |
| `maxFullSwitchCandidates` | 3 | Trainer reserves receiving contextual evaluation |
| `maxSearchedSwitchActions` | 2 | Switches admitted to final four AI actions |
| `maxFutureAiActions` | 3 | Expensive future subset |
| `maxFuturePlayerActions` | 3 | Expensive future subset |
| `futureValuePercent` | 50–60% | Discount for the best follow-up action |
| `planTurnCap` | 6 | Saturation cap for analytic turns-to-KO comparison |
| `failedReadRepeatPenaltyTurns` | 1–2 | Short-term identical-read dampening |
| `readBalanceClamp` | -8..+8 | Prevent global calibration runaway |

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

/* Resident owner of transient lifetime. */
BOOL AI_RunTransientPlanner(const AIPlannerRequest *request,
                            AIPlannerResult *result);

/* Sole transient entrypoint at 0x023C0400. */
u32 TrainerAIPlanner_Entry(const AIPlannerRequest *request,
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
2. Player action generation ranks the exact legal attacks and both switch
   destinations. The most plausible four become primary branches; the rest
   retain compact tail identities.
3. Trainer switch screening retains the safest pivot, strongest attacker and
   best support/setup reserve, then admits at most two switch actions to the
   final four trainer actions.
4. Thundershock receives a catastrophic outcome when the current Ground target
   stays and a favorable outcome only for switch destinations it can hit.
5. If safe neutral coverage reaches the robust target, Thundershock is not
   eligible for safe near-best sampling.
6. If no robust action exists and the switch probability/payoff genuinely
   justifies Thundershock, selecting it creates a prediction commitment.
7. The next decision resolves whether the player actually switched and whether
   another feasible action would have covered the result. A failed specialized
   read discourages an immediate identical repeat.

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
> lists each Section-3 regression requirement before implementing v0.5.
