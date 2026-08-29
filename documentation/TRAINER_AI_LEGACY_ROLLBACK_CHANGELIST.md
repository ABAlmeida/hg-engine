# Legacy Trainer AI Rollback Changelist

This changelist describes the source-level rollback required before implementing
`TRAINER_AI_TECHNICAL_DESIGN.md` v0.5.

| Status | Value |
|---|---|
| Source rollback | Applied 2026-08-29 |
| Static reference review | Complete |
| Build/size measurement | Pending explicit authorization |
| Runtime verification | Not performed |

The legacy implementation was introduced by commit
`ad0d002a1f4dc2eb20782391bdfb39ec58ebee17` (`Implement fair-information
expert trainer AI`). Do **not** apply a whole-commit inverse. That commit also
contains a general move-data bug fix and trainer-data normalization that must
remain.

## Removed legacy runtime

| Path | Changelist action |
|---|---|
| `src/battle/trainer_ai.c` | Delete the legacy fair-information evaluator, its static knowledge/cache state, command picker and move picker. |
| `include/trainer_ai.h` | Delete the legacy runtime interface. |
| `include/config.h` | Remove `IMPLEMENT_FAIR_TRAINER_AI` and its legacy description. The v0.5 implementation must introduce a newly named guard or deliberately redefine this one only when the replacement exists. |
| `hooks` | Remove the guarded `FairTrainerAI_PickCommand` and `FairTrainerAI_PickMove` overlay-12 hooks. This temporarily restores direct use of the original HGSS command/move AI. Record the verified call-site addresses for v0.5 rather than leaving dead hooks installed. |
| `rom.ld` | Remove `TrainerAI_PickMove_Original` and `TrainerAI_PickCommand_Original` if no remaining source references them after the legacy module is deleted. Re-add only the symbols the v0.5 resident wrapper actually calls. |
| `src/battle/battle_start.c` | Remove the guarded `trainer_ai.h` include and `FairTrainerAI_Reset()` call. Preserve all unrelated battle initialization. |
| `src/battle/battle_script_commands.c` | Remove the guarded include and legacy held-item/ability observation calls. Preserve Knock Off and Ability Popup mechanics themselves. Exact-information v0.5 state will be refreshed through its central sanitizer instead. |
| `src/battle/battle_item.c` | Remove the guarded include and legacy held-item observation call. Preserve held-item activation behavior. |

After these edits, a repository search for `FairTrainerAI_`,
`TrainerAI_UsesStrategicLayer` and `IMPLEMENT_FAIR_TRAINER_AI` must return no
runtime references. Generated objects, binaries and ROM output do not count as
source references.

## Preserve deliberately

| Path or behavior | Reason |
|---|---|
| `src/moves.c`: `MOVE_DATA_PSS_SPLIT` returns `bm->split` | This is a general move-data correctness fix. Restoring `bm->effect` would be a regression unrelated to AI removal. |
| `include/trainer_data.h`: `F_TRAINER_EXPERT_AI` | The alias predates the legacy AI commit and is the production content profile. Without a custom hook it correctly expands to the original HGSS Super Effective, Evaluate Attacks and Expert Attacks modules. |
| `data/Trainers.c` expert-profile assignments | They are content authoring decisions, not ownership of the removed evaluator. Later trainer-balance commits also depend on them. Preserve extra flags such as `F_USE_WEATHER`, `F_PRIORITIZE_STATUS_MOVES` and `F_MULTI_BATTLE_PARTNER`. |
| Original HGSS AI and unsupported-format paths | These form the clean transitional baseline and the permanent doubles/unsupported fallback for v0.5. |
| All unrelated battle, item, trainer and progression changes | The rollback is scoped to the legacy custom AI only. |

Configured trainer Bag items are not removed. During the temporary original-AI
baseline, HGSS may use configured trainer items. The v0.5 expert singles path
must never generate or select ITEM; that behavior belongs to the replacement,
not to this rollback.

## Documentation changes applied with the rollback

- Make `TRAINER_AI_TECHNICAL_DESIGN.md` the authoritative replacement design.
- Mark `TRAINER_AI_PLAN.md` and `TRAINER_AI_DOUBLES_PLAN.md` as legacy or
  superseded; do not let their fair-information behavior override v0.5.
- Update `PROJECT_PLAN.md` so it does not claim the removed contextual AI is
  active.
- Keep the trainer-authoring guidance for `F_TRAINER_EXPERT_AI`, but explain
  that it maps to the original HGSS modules until v0.5 is implemented.
- Retain old `CODE_BUDGET.md` values as dated historical evidence. Add fresh
  post-rollback measurements only after an explicitly authorized build.

## Verification state

The source rollback is complete only when all of the following hold:

1. The source-reference search described above is clean.
2. `F_TRAINER_EXPERT_AI` and all intended trainer assignments remain.
3. `GetMoveData(id, MOVE_DATA_PSS_SPLIT)` still returns `bm->split`.
4. No v0.5 transient-overlay sources, overlay table entry or generated patch
   inputs remain from the discarded attempt.
5. A source diff contains no unrelated trainer/content reversal.
6. When the user explicitly authorizes a build, a clean controlled build must
   still establish the post-rollback overlay-130 baseline and ROM behavior.

Do not run a build, test suite or emulator merely to complete this documentation
changelist. Building remains a separately authorized action.
