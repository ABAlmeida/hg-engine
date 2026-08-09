# Fair-Information Trainer AI Plan

Last updated: 2026-08-09

## Status

Planned. No AI implementation from this plan has started.

## Objective

Build the strongest practical trainer AI that fits Heartless Gold's Nintendo
DS runtime and memory constraints without reading information an opposing
player could not know. Preserve hg-engine's existing move-scoring modules and
battle mechanics, then improve the command, switching, prediction, and
long-term decision layers around them.

This is a partially observable and stochastic game. A provably optimal AI is
not realistic on this hardware, but a strong bounded-search AI with explicit
knowledge, uncertainty, prediction, and team-role heuristics is feasible.

## Expert-profile rule

`F_TRAINER_EXPERT_AI` is the single opt-in profile. It currently expands to:

```c
F_PRIORITIZE_SUPER_EFFECTIVE |
F_EVALUATE_ATTACKS |
F_EXPERT_ATTACKS
```

Whenever a trainer needs exactly that combination, use the alias rather than
spelling out its component flags. If the profile later gains fair switching or
other behaviour, every trainer using the alias must receive it automatically.

Do not add an unverified new bit such as `F_SMART_SWITCHING` to trainer data.
AI flag bits select compiled move-scoring scripts directly; a new bit requires
a corresponding valid script entry and interpreter audit. The strategic layer
should initially activate by testing that the expert-profile bits are present.

During implementation, mechanically replace existing exact three-flag
expressions with `F_TRAINER_EXPERT_AI`. Preserve entries that intentionally add
weather, doubles, healing, harassment, or another strategy module.

## Fair-information contract

### Information the AI may know immediately

- Its complete party, moves, abilities, held items, stats, status, and plan.
- Public field, weather, terrain, side-condition, hazard, turn, and battle-type
  state.
- The active player's visible species, form, level, gender, displayed status,
  stat stages, and coarse HP-bar state.
- The number of remaining player Pokémon where the normal battle UI exposes
  that count, but not their unrevealed species.
- Deterministic game rules, type data, species base data, and natural
  level-up learnsets.

### Information that must begin unknown

- Unrevealed player party species and forms.
- The player's unrevealed moves.
- TM, HM, tutor, Egg, event, or otherwise non-level-up moves.
- The player's exact ability before it is revealed. Prediction may consider
  normal ability slots but never a hidden ability.
- The player's held item before the item or its effect is revealed.
- Exact IVs, EVs, nature, internal stats, exact HP number, personality, and
  other private Pokémon data.
- The command currently selected by the player.
- Future RNG results.

### Information learned during battle

- A move after that move is used or otherwise publicly revealed.
- An ability after its activation, message, suppression, copying, swapping, or
  another public effect identifies it.
- A held item after activation, consumption, removal, theft, swapping, or
  another public effect identifies it.
- A Pokémon's species and form after it enters battle.
- Its last publicly visible HP range, status, stat stages, and whether it has
  fainted when it leaves the field.
- Relative speed evidence from action order, accounting for known priority,
  paralysis, Tailwind, Trick Room, and similar public modifiers.
- Approximate offensive or defensive ranges inferred from observed damage;
  never the actual hidden stats stored in the player's party.

Once a hidden ability, unusual move, or item is genuinely revealed, it becomes
known even if it could not have been predicted initially.

## Natural prediction model

The AI may make uncertain predictions rather than treating unknown information
as empty.

### Moves

For a revealed species and level:

1. Read that species' level-up learnset.
2. Include moves learned naturally at or below the visible level, including
   level-1 and evolution moves where the engine's learnset represents them.
3. Give the most recent plausible four moves the greatest weight, while
   retaining lower weight for older level-up moves the player could have kept.
4. Give an actually revealed move certainty regardless of its acquisition
   method.
5. Never predict an unrevealed TM, HM, tutor, Egg, event, or custom move.

The first version should use the current species' own learnset only. Inferring
pre-evolution-only moves can be considered later, but it must remain based on a
verified legal natural path rather than silently expanding to arbitrary moves.

### Abilities

- Predict only the species' ordinary ability slots.
- Split confidence when two normal abilities are possible.
- Never predict the hidden ability before evidence reveals it.
- Collapse the prediction to the exact ability after a public reveal.

### Held items and reserve Pokémon

- Do not predict an unrevealed held item.
- Do not invent an unrevealed reserve species.
- Predict switches only into previously revealed, non-fainted reserve Pokémon.
- An unknown reserve count may affect the general likelihood of a switch, but
  never create a fictional matchup or immunity.

Use compact integer weights rather than floating point. Unknown possibilities
must reduce confidence and encourage robust choices rather than give the AI
secret certainty.

## Architecture

### 1. Preserve and audit the existing AI

The current trainer flags select compiled move-scoring scripts in battle
overlay 10. `armips/asm/trainer_ai.s` patches their expanded move-table access,
but the individual HGSS modules are not maintained as readable source in this
repository.

Before changing their behaviour:

1. Identify the exact US HGSS command picker, switch evaluator, AI interpreter,
   module table, and module ranges.
2. Document every AI command that reads the opponent's move, ability, item,
   stats, party, or current action.
3. Confirm all addresses and instructions against the supported ROM revision.
4. Add guarded Armips assertions at any required hook site.
5. Use the current pret HeartGold reconstruction as primary HGSS evidence.
   Platinum may explain shared Gen-IV logic but must not establish an HGSS
   address, structure, parameter, or behaviour by assumption.

Long term, reconstruct the HGSS AI bytecode as readable source and reproduce
the original bytes before changing it. Direct opaque byte edits are acceptable
only for a small, fully documented, same-size correction.

### 2. Add battle-scoped knowledge memory

Create one compact record for each player party slot that has been revealed.
It should contain only learned/public state, for example:

- revealed species and form;
- up to four revealed move IDs;
- known ability or unknown;
- known held item or unknown;
- last visible HP bucket, status, and fainted state;
- confidence/range evidence needed for speed or damage inference.

Reset it at battle initialization and discard it with the battle overlay. It
must not enter save data. Prefer battle-extension BSS or an existing
battle-lifetime allocation; do not consume the nearly full permanent
overlay-129 region.

All new expert-AI queries about the player must go through a narrow knowledge
API. Code review should reject direct reads of private player party data from
the strategic evaluator.

### 3. Record public observations centrally

Use the smallest verified set of battle lifecycle points:

- send-out or form reveal;
- move announcement/execution;
- ability announcement or public ability effect;
- held-item announcement, activation, consumption, removal, or swap;
- damage and action-order resolution;
- switch-out and faint completion.

One observer should update knowledge for all expert decisions. Avoid separate
hooks for each heuristic. Every hook must preserve original control flow and
must execute only while the battle extension is loaded.

### 4. Add productive-action and switching evaluation

Extend the existing Fight/Item/Switch picker before replacing any move-scoring
module. Activate it for `F_TRAINER_EXPERT_AI`; all other trainers retain the
original path.

The evaluator must:

- preserve every switching legality rule;
- enumerate living legal reserve slots without colliding with a doubles
  partner's active or already-selected slot;
- recognise damaging moves that are disabled, out of PP, immune, or otherwise
  unusable;
- recognise useful support moves rather than treating all status-only Pokémon
  as dead weight;
- switch when every remaining action is redundant, blocked, or materially
  worse than a reserve's action;
- require a meaningful score improvement to avoid switch loops.

For the current Whimsicott case, Tailwind, Light Screen, Leech Seed, and Poison
Powder are productive only while their relevant public condition can still be
changed. After useful support is established or blocked, the AI should prefer a
legal reserve that can advance the battle.

### 5. Score the trainer's own win condition

Infer roles from the trainer's known team rather than from trainer class:

- immediate attacker or cleaner;
- setup attacker;
- speed, screen, weather, terrain, or hazard support;
- status and disruption support;
- defensive pivot;
- matchup-specific counter.

Use these roles to value:

- remaining material and usable HP;
- immediate and expected KOs;
- speed control, screens, hazards, weather, terrain, status, and setup;
- preservation of the only revealed answer to an opposing threat;
- safe pivots and sacrifice decisions;
- endgame conditions where preserving a support Pokémon no longer matters.

This is a generic team-derived objective. Avoid trainer-specific C branches or
hardcoded species plans. Content-specific personality can later be expressed
through data weights if needed.

### 6. Predict the player's next action

Build a probability distribution over legal-looking actions using only the
knowledge model:

- revealed moves receive the highest confidence;
- plausible natural level-up moves receive lower confidence;
- likely KO, super-effective, STAB, priority, recovery, setup, and status
  actions receive contextual weight;
- impossible, publicly disabled, or exhausted moves receive zero weight;
- switching becomes more likely when the active Pokémon is threatened, cannot
  make progress, or has a previously revealed safe reserve;
- staying becomes more likely when the player has a strong visible action or
  when all revealed switches are poor.

The AI must never inspect `playerActions` or an equivalent current-turn input
buffer. Prediction must be computed before and independently of the player's
actual selection.

### 7. Use bounded action search

For singles, evaluate each trainer action against the weighted player action
distribution:

```text
expected score = immediate outcome
               + resulting position
               + win-condition preservation
               - risk and switch cost
```

Use fixed-point integer arithmetic. Start with one-turn expectimax and a small
worst-case component so the AI does not choose an action that wins only against
one optimistic prediction.

Bound the work by pruning to the strongest predicted player actions and legal
revealed switches. Do not perform unbounded game-tree search. A later second
ply may consider only the top few action pairs after profiling proves it fits
the frame budget.

### 8. Treat doubles as a later phase

First ship and verify singles. Doubles require joint decisions for both active
trainer Pokémon, target coordination, spread moves, ally support, redirection,
and reserve-slot arbitration. Until that phase is implemented, expert doubles
must retain the existing doubles AI rather than using a singles evaluator in an
unsafe partial form.

## Logical implementation phases

1. **Baseline and audit** — identify exact HGSS functions, data, module ranges,
   hidden-information reads, hook sites, object sizes, and performance budget.
2. **Profile normalization** — replace exact three-flag trainer expressions
   with `F_TRAINER_EXPERT_AI`; preserve intentional extra flags.
3. **Knowledge contract** — add battle-scoped records, accessors, reset, and
   public species/move/ability/item observation.
4. **Fairness barrier** — route expert opponent-information queries through
   knowledge or prediction and prevent current-input/private-party reads.
5. **Singles switching** — productive support detection, legal reserve
   enumeration, candidate scoring, and anti-loop thresholds.
6. **Natural prediction** — cached level-up move and normal-ability candidate
   generation with explicit uncertainty.
7. **Player action model** — move and revealed-switch probability scoring.
8. **Win-condition evaluator** — role inference, positional value, preservation,
   and endgame priorities.
9. **Bounded singles search** — one-turn expected-value selection, pruning, and
   performance instrumentation.
10. **Doubles coordination** — paired action/target/switch evaluation after the
    singles system is stable.
11. **Source reconstruction and tuning** — convert inherited HGSS AI modules to
    maintainable source where useful, then tune weights through documented
    gameplay evidence rather than trainer-specific exceptions.

Each phase must remain independently reviewable and must fall back to original
HGSS behaviour when its requirements are not satisfied.

## Feasibility and cost

| Capability | Feasibility | Main risk |
| --- | --- | --- |
| Expert-profile normalization | Very high | Large mechanical trainer-data diff must preserve entries with extra flags. |
| Status-only and matchup switching | High | Correct support-move usefulness and trapping rules. |
| Revealed-information memory | High | Finding minimal reliable observation points without lifecycle bugs. |
| Natural move/normal-ability prediction | High | Efficient learnset access and representing uncertainty compactly. |
| Fair held-item/ability handling | Medium-high | Auditing inherited AI commands that may read actual hidden fields. |
| Player move prediction | Medium-high | Predictions are necessarily uncertain and require tuning. |
| Player switch prediction | Medium | Unrevealed reserves must remain unknown, limiting certainty by design. |
| Generic win-condition recognition | Medium | Role and preservation heuristics must work across varied teams. |
| One-turn bounded search | Medium-high | Runtime profiling and avoiding duplicate damage calculations. |
| Strong coordinated doubles | Medium-low | Joint-action branching and many interaction-specific rules. |
| Provably optimal play | Not feasible | Pokémon is stochastic, partially observable, and too large for exhaustive DS search. |

The recommended practical target is a strong, explainable, fair singles AI
with bounded prediction, followed by doubles. That is feasible. A competitive
simulator-scale search engine is not appropriate for this ROM and hardware.

### Preliminary memory estimate

These are planning estimates, not measured results:

| Component | Expected cost |
| --- | ---: |
| Knowledge records and transient state | 100-250 bytes of battle-lifetime BSS |
| Observation and fair-query layer | 1-2 KB battle-extension code |
| Productive-action and switching evaluator | 0.7-1.5 KB battle-extension code |
| Natural prediction and cached candidates | 1-2 KB code plus small transient cache |
| Singles action model and bounded evaluator | 2-4 KB battle-extension code |
| Later doubles coordination | 1-3 KB additional code |

The last recorded battle-extension use was 72,192 of 81,920 bytes, leaving
9,728 bytes. The complete ambition may approach that headroom. Measure after
every phase, keep lookup tables in packaged data where practical, and reclaim
or relocate code before reducing safety margins. Do not place this system in
overlay 129.

Avoid per-turn NARC loads or heap churn. Cache compact predictions when a
species is first revealed and invalidate them only when new evidence arrives.
The synchronous decision target should remain comfortably below one frame;
profile actual cycles before adding another search ply.

## Testability and diagnostics

Keep evaluators as small deterministic functions with explicit inputs. Add a
compile-time `DEBUG_TRAINER_AI` trace that can report, without changing normal
builds:

- currently known and predicted player information with confidence weights;
- legal trainer actions and rejected switch candidates;
- predicted player action distribution;
- score components for each retained action;
- the final action and tie-break reason;
- decision time or cycle budget.

Focused manual scenarios must cover:

- a status-only lead before and after its support becomes redundant;
- no legal reserve, trapped, Ingrain, and ability-based trapping;
- immunity, resistance, neutral, and super-effective reserve choices;
- revealed versus merely predicted moves and abilities;
- a TM/HM move and hidden ability remaining unknown until revealed;
- an unrevealed held item remaining unknown until activation;
- prediction of attack versus switch without reading the actual input;
- prevention of two-Pokémon switch loops;
- unknown reserves never appearing in predictions;
- save-independent reset across consecutive battles;
- unchanged behaviour for trainers without `F_TRAINER_EXPERT_AI`;
- deterministic singles before any doubles path is enabled.

Heartless Gold tests and emulator execution remain subject to the repository
skill restrictions. A successful build proves only compilation and packaging,
not AI correctness or fairness.

## Completion criteria

- Every exact three-module expert trainer uses `F_TRAINER_EXPERT_AI`.
- Expert decisions never inspect the player's current command or unrevealed
  party, held item, TM/HM move, or hidden ability.
- All opponent knowledge is either public, observed, or represented as an
  explicitly uncertain legal natural prediction.
- Status-only support Pokémon act while useful and switch when they can no
  longer advance their plan.
- Switching is legal, materially beneficial, and resistant to loops.
- The AI preserves and pursues team-derived win conditions without
  species-specific code branches.
- Singles prediction and search stay within the measured runtime and battle
  extension budgets.
- Non-expert, wild, roaming, Safari, catching-demo, and unimplemented doubles
  paths preserve their existing behaviour.
- Documentation, debug traces, hook assertions, code-budget measurements, and
  focused manual verification results are current.
