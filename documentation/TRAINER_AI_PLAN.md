# Fair-Information Trainer AI Plan

Last updated: 2026-08-09

## Status

Contextual fair strategic layer implemented on 2026-08-09. The initial
`quick-rom` build succeeded; the subsequent reusable-support correction is
implemented in source but not built because that change was not accompanied by
an explicit build request. Focused manual verification is pending. Doubles-specific
coordination is deliberately excluded and tracked in
`TRAINER_AI_DOUBLES_PLAN.md`.

The implemented layer covers the verified command/move hook sites, central
profile gate, battle-local reset, public active-Pokemon/move/ability/item
observations, current-species level-up move and ordinary-ability prediction,
contextual attack, status, stat-stage, recovery, screen, hazard, speed-control,
survival, and disruption checks; legal reserve scoring; switch hysteresis;
bounded player attack/switch scenarios; and one-roll response sampling.
Unsupported effects trigger a fair whole-decision fallback instead of being
silently removed or assigned fake attack power. Non-expert trainers retain the
original HGSS pickers.

Reserve scoring now uses the candidate's best contextual action on the next
turn rather than only its best damaging move. The switch consumes the current
turn and charges predicted entry damage; only a candidate expected to survive
receives discounted next-action value. Tailwind, Reflect, and Light Screen are
valued across living teammates whose speed matchup or expected damage changes,
with reserve benefits discounted by the temporary effect's lifetime. This lets
support Pokemon leave when their current jobs are complete and become useful
again when those field effects expire or a new status/Leech Seed target enters,
without hardcoding a trainer or species.

Command caching includes the active party slot as well as the turn number, so
a replacement cannot inherit the fainted Pokemon's cached command when both
events share a turn counter. The AI also records whether the current slot has
chosen an action since entering. Initial leads remain unrestricted, but every
later forced, voluntary, or effect-driven entry must choose one non-switch
command before another voluntary switch. A candidate with no usable move may
still switch as an emergency legality fallback. This prevents zero-progress
switch chains without imposing a multi-turn lock or trainer-specific rule.

Remaining follow-up work is explicit rather than silently inferred: add a
compact build-time pre-evolution lineage source, audit additional public item
and ability reveal paths, add damage/speed evidence and debug timing output,
and reconstruct an opaque HGSS module only if tuning proves the replacement
layer cannot preserve a useful behavior fairly. Coordinated doubles strategy
is not part of those completion criteria.

### Implemented integration map

- Overlay-12 Thumb call `0x0225E10E`, originally targeting HGSS command picker
  `0x022205BC`, now targets `FairTrainerAI_PickCommand`.
- Overlay-12 Thumb call `0x0225E43E`, originally targeting HGSS move picker
  `0x0221BEF4`, now targets `FairTrainerAI_PickMove`.
- The hook installer verifies that both replaced instructions are Thumb `BL`
  calls. Their original targets remain named in `rom.ld` and are used for every
  trainer that does not have the complete expert profile. Those absolute Thumb
  targets are declared `LONG_CALL`, forcing odd-address `BLX` calls instead of
  invalid ARM interworking veneers.
- The implementation and its 280-byte knowledge state live in battle extension
  overlay 130. `ServerInit` clears that state for every battle, so knowledge
  cannot leak through heap-address reuse or save data.
- Used moves are accumulated from public previous-move/AI observation fields.
  Ability popups and activating or removed held items update the same knowledge
  record at their public message/effect points.
- Expert scoring reads visible active-Pokemon state and the trainer's own party.
  It does not call the opaque HGSS expert move picker, read `playerActions`, or
  inspect the player's unrevealed party, moves, item, or ability. Actual
  opposing trapping abilities are consulted only as a legality oracle so the
  AI cannot submit an impossible switch command.

## Objective

Build the strongest practical trainer AI that fits Heartless Gold's Nintendo
DS runtime and memory constraints without reading information an opposing
player could not know. Preserve hg-engine's battle mechanics and the original
picker for non-expert profiles, while expert trainers use the maintainable
fair-information command, switching, prediction, and contextual move layer.

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

Add a global `IMPLEMENT_FAIR_TRAINER_AI` configuration guard and one central
`TrainerAI_UsesStrategicLayer(aiFlags)` helper. The helper returns true when
all `F_TRAINER_EXPERT_AI` bits are present, including when a trainer also uses
additional modules. This does not consume another serialized AI-module bit.
The guard makes the whole layer removable for comparison or emergency fallback;
adding or removing `F_TRAINER_EXPERT_AI` remains the per-trainer control.

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

Include verified natural moves from the current species and its legal
pre-evolution lineage. Follow only evolution paths that can lead to the
revealed species, and include only level-up and explicit evolution moves. If
the engine has no cheap reverse-evolution helper, generate or package a compact
lineage lookup once at build time rather than scanning every species during an
AI turn. Branched relatives that cannot lead to the revealed species must not
contribute moves.

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

Every expert-AI query about the player must go through a narrow knowledge API,
including queries made by inherited HGSS move-scoring modules. Where an opaque
interpreter command currently reads the real opposing move, ability, held item,
party member, or private stat, intercept that command for expert trainers and
return `known`, `predicted`, or `unknown` information instead. Code review must
reject direct private-player reads anywhere on the expert decision path, not
only in newly written evaluators.

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

Extend the existing Fight/Switch picker before replacing any move-scoring
module. Activate it through `TrainerAI_UsesStrategicLayer`; all other trainers
retain the original path. Do not add trainer Bag-item evaluation: Heartless
Gold content is being changed so trainers have no configured Bag items. Held
items remain normal battle state and must still be evaluated when known.

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

### 7. Use bounded evaluation and scenario-weighted response sampling

Evaluate each trainer action against the weighted player-action distribution:

```text
expected score = immediate outcome
               + resulting position
               + win-condition preservation
               - risk and switch cost
```

Use fixed-point integer arithmetic and a bounded one-turn outcome model. For
each retained player scenario, identify the best non-dominated trainer response
and assign that response the scenario's probability mass. Combine mass when
several player scenarios select the same trainer response, remove responses
that are materially inferior across every scenario, normalize the remaining
weights, and use the battle RNG to select the trainer action.

For example, if the model assigns 30% to a player switch whose best response is
an attacking move, and 70% to a player attack whose best response is a trainer
switch, the resulting trainer choice is approximately 30% attack and 70%
switch. This is deliberate probability matching, not random selection among
all legal actions. An action that is dominated or catastrophically bad must not
receive weight merely to create variety.

Bound the work by pruning to the strongest predicted player actions and legal
revealed switches. Do not perform unbounded game-tree search. A later second
ply may consider only the top few action pairs after profiling proves it fits
the frame budget.

### 8. Explicit battle-format scope

Singles are the correctness and tuning target. The generic knowledge,
prediction, and per-battler response layer may also run in double or multi
battles when `F_TRAINER_EXPERT_AI` is present, but this plan adds no paired
action search, ally coordination, redirection strategy, spread-move strategy,
or joint target optimization. Double-battle choices may therefore be less
accurate, but must remain legal, bounded, and crash-free. Any improvement beyond
that is separately optional in `TRAINER_AI_DOUBLES_PLAN.md`.

## Logical implementation phases

Each phase is a separate review and measurement boundary. Do not combine the
initial audit with behavioural changes.

### Phase 1: establish the verified HGSS baseline

Deliverables:

- identify and document the US HGSS command picker, switch evaluator, AI
  interpreter, module pointer table, module ranges, RNG call, and lifecycle;
- map every opponent-information and current-command read used by those paths;
- record exact hook instructions, Thumb/ARM state, continuation addresses,
  structure offsets, and overlay lifetime;
- record the current battle-extension link size and a representative decision
  timing baseline; and
- decide from evidence whether each change is best made in reconstructed HGSS
  code, an asserted Armips hook, or battle-extension C.

Exit criterion: every proposed entry point and private-information read is
verified against HGSS. Platinum evidence alone cannot satisfy this phase.

### Phase 2: normalize and gate the expert profile

Deliverables:

- add `IMPLEMENT_FAIR_TRAINER_AI` to the established configuration surface;
- expose it to Armips through the existing generated configuration path if an
  asserted binary hook needs the same guard; do not create a second hand-kept
  setting;
- add `TrainerAI_UsesStrategicLayer(aiFlags)` as the only activation check;
- replace exact three-flag expressions in `data/Trainers.c` with
  `F_TRAINER_EXPERT_AI`, preserving all deliberate extra flags; and
- prove that disabling the configuration guard leaves the original decision
  path intact.

Exit criterion: source has one recognizable per-trainer opt-in and one global
fallback, with no new serialized AI-module bit.

### Phase 3: add the knowledge model and fairness barrier

Deliverables:

- define compact battle-lifetime knowledge records and explicit unknown values;
- initialize and clear them at verified battle lifecycle points;
- record public species/form, moves, abilities, held items, HP buckets, status,
  fainting, and useful action-order evidence;
- expose narrow `Known`, `Observed`, and `Predicted` query functions;
- route new strategic queries through those functions; and
- intercept, replace, or reconstruct every inherited expert-module query that
  the phase-1 audit identifies as reading private player state.

Exit criterion: neither the new layer nor inherited modules on the expert path
can obtain the player's party object, current command buffer, exact hidden
stats, or unrevealed item/ability. Consecutive battles cannot share knowledge.

### Phase 4: implement productive actions and switching

Deliverables:

- enumerate legal trainer moves and reserve switches;
- score whether damaging and support moves can still advance the position;
- score reserve matchups using only fair knowledge;
- add hysteresis, recent-switch memory, and a minimum improvement threshold to
  prevent loops; and
- retain legal per-battler behaviour in doubles without adding coordination.

Exit criterion: the Whimsicott scenario uses useful support while relevant and
then selects an attacking reserve when support becomes redundant, while
trapping, no-reserve, disabled-move, immunity, and switch-loop cases remain
legal.

### Phase 5: add natural move and ability prediction

Deliverables:

- obtain the current species and verified ancestor chain without a per-turn
  all-species scan;
- generate weighted current-species and ancestor level-up/evolution moves;
- generate only ordinary ability-slot candidates;
- cache candidates on reveal and invalidate only when evidence changes; and
- replace a prediction with certainty after the actual move or ability is
  revealed.

Exit criterion: no unrevealed TM, HM, tutor, Egg, event, custom move, hidden
ability, or held item enters the candidate set.

### Phase 6: model player actions

Deliverables:

- assign fixed-point probabilities to revealed and predicted moves;
- model switching only to revealed, living reserves while allowing an unknown
  reserve count to affect general switch likelihood;
- account for visible matchup, likely KO, priority, STAB, recovery, setup,
  status, inability to progress, and public battle state; and
- normalize the retained player scenarios to a documented fixed total.

Exit criterion: the distribution is derived without reading the current player
selection and debug output can explain every nonzero scenario weight.

### Phase 7: infer and score the trainer's win condition

Deliverables:

- infer generic attacker, cleaner, setup, support, disruption, pivot, and
  matchup-answer roles from the trainer's known team;
- value material, usable HP, KO pressure, speed control, screens, hazards,
  weather, terrain, status, setup, preservation, sacrifice, and endgame state;
- keep weights centralized and named; and
- prohibit trainer-ID and species-specific strategy branches.

Exit criterion: score components are explainable and the AI preserves uniquely
valuable answers without refusing necessary endgame sacrifices.

### Phase 8: add bounded response selection

Deliverables:

- calculate one-turn outcomes for retained trainer/player action pairs;
- discard illegal and dominated trainer actions;
- assign each player scenario's probability mass to its best acceptable trainer
  response, combine and normalize response weights, then make one RNG roll;
- bound candidate counts and reuse cached calculations; and
- emit decision time, scenario weights, action scores, rejection reasons, and
  the final random roll under `DEBUG_TRAINER_AI`.

Exit criterion: probability matching behaves as documented, materially bad
actions receive no artificial weight, and the measured decision remains within
the agreed runtime and memory headroom.

### Phase 9: reconstruct and tune only where justified

Deliverables:

- reconstruct an opaque HGSS module only when the audit shows that its hidden
  reads, behaviour, or maintenance cost cannot be addressed safely outside it;
- reproduce original bytes/behaviour before applying semantic changes;
- replace opaque patches with named source and assertions where practical; and
- tune generic weights from documented manual scenarios, not one-off trainer or
  species exceptions.

Exit criterion: reconstructed code is evidence-backed and each retained binary
hook remains asserted, minimal, and documented.

Likely touched paths are `include/config.h`, `include/trainer_data.h`,
`include/battle.h`, `data/Trainers.c`, `src/battle/ai.c`, `src/overlay.c`,
`armips/asm/trainer_ai.s`, `rom.ld`, and this documentation. Phase 1 must refine
that list before implementation changes begin.

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
| Scenario-weighted response sampling | Medium-high | Runtime profiling, probability calibration, and avoiding duplicate damage calculations. |
| Provably optimal play | Not feasible | Pokémon is stochastic, partially observable, and too large for exhaustive DS search. |

The recommended practical target is a strong, explainable, fair singles AI
with bounded prediction and legal but uncoordinated reuse in doubles. That is
feasible. A competitive simulator-scale search engine is not appropriate for
this ROM and hardware. Coordinated doubles is not part of this estimate.

### Preliminary memory estimate

These are planning estimates, not measured results:

| Component | Expected cost |
| --- | ---: |
| Knowledge records and transient state | 100-250 bytes of battle-lifetime BSS |
| Observation and fair-query layer | 1-2 KB battle-extension code |
| Productive-action and switching evaluator | 0.7-1.5 KB battle-extension code |
| Natural prediction and cached candidates | 1-2 KB code plus small transient cache |
| Action model and bounded response evaluator | 2-4 KB battle-extension code |

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

Keep scoring and probability construction as small deterministic functions with
explicit inputs; isolate the final RNG selection so fixed seeds are
reproducible. Add a compile-time `DEBUG_TRAINER_AI` trace that can report,
without changing normal builds:

- currently known and predicted player information with confidence weights;
- legal trainer actions and rejected switch candidates;
- predicted player action distribution;
- score components for each retained action;
- normalized response weights, the random roll, and the final action;
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
- legal, crash-free per-battler decisions in doubles without claiming
  coordinated doubles quality.

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
- Prediction and response selection stay within the measured runtime and battle
  extension budgets.
- Non-expert, wild, roaming, Safari, and catching-demo paths preserve their
  existing behaviour.
- Doubles remain legal and bounded; coordinated doubles quality is explicitly
  deferred rather than treated as a completion requirement.
- Documentation, debug traces, hook assertions, code-budget measurements, and
  focused manual verification results are current.
