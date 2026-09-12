#include "battle.h"
#include "ai_config.h"
#include "config.h"
#include "pokemon.h"
#include "trainer_ai.h"
#include "trainer_data.h"
#include "types.h"
#include "constants/ability.h"
#include "constants/generated/trainer_ai_semantics_generated.h"
#include "constants/battle_constants.h"
#include "constants/file.h"
#include "constants/hold_item_effects.h"
#include "constants/move_effects.h"
#include "constants/moves.h"
#include "constants/species.h"

#ifdef IMPLEMENT_EXPERT_TRAINER_AI

#define FAIR_AI_PARTY_SIZE           6
#define FAIR_AI_PREDICTED_MOVES      MAX_MON_MOVES
#define FAIR_AI_MAX_ACTIONS          9
#define FAIR_AI_FUTURE_PERCENT       AI_CFG_SPECULATIVE_FUTURE_PERCENT
#define FAIR_AI_INVALID              (-30000)

#define FAIR_AI_PLAYER_ACTION_MOVE   0
#define FAIR_AI_PLAYER_ACTION_SWITCH 1

#define FAIR_AI_MOVE_STATE_FORCED    (1 << 0)
#define FAIR_AI_MON_STATE_FLASH_FIRE (1 << 0)

typedef struct FairAIKnownMon {
    u16 species;
    u16 moves[FAIR_AI_PREDICTED_MOVES];
    u16 ability;
    u16 item;
    u16 hp;
    u16 maxHp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 spAttack;
    u16 spDefense;
    u8 moveCount;
    u8 form;
    u8 level;
    u8 hpPercent;
    u8 type1;
    u8 type2;
    u8 pp[MAX_MON_MOVES];
    u8 stages[8];
    u32 condition;
    u32 condition2;
    u32 effectFlags;
    u8 semanticState;
} FairAIKnownMon;

typedef FairAIKnownMon FairAIMonModel;

typedef enum FairAIActionKind {
    FAIR_AI_ACTION_MOVE,
    FAIR_AI_ACTION_SWITCH,
} FairAIActionKind;

typedef struct FairAIAction {
    s16 fastScore;
    s16 score;
    s16 pressureScore;
    u8 kind;
    u8 index;
} FairAIAction;

typedef struct FairAIPlayerAction {
    u16 value;
    u8 probability;
    u8 kind;
} FairAIPlayerAction;

typedef struct FairAIMoveState {
    u16 forcedMove;
    u16 previousMove;
    u8 forcedProgress;
    u8 repeatProgress;
    u8 flags;
    u8 reserved;
} FairAIMoveState;

typedef struct FairAIDecision {
    u16 turn;
    u8 valid;
    u8 activeSlot;
    u8 command;
    u8 move;
    u8 target;
    u8 switchSlot;
} FairAIDecision;

typedef struct FairAICommitment {
    u16 turn;
    u8 valid;
    u8 playerBattler;
    u8 playerSlot;
    u8 situation;
    u8 actionClass;
} FairAICommitment;

typedef struct FairAIState {
    FairAIKnownMon playerMons[FAIR_AI_PARTY_SIZE];
    FairAIDecision decisions[CLIENT_MAX];
    FairAICommitment commitments[CLIENT_MAX];
    AIPredictionCase recentCases[AI_CFG_RECENT_BATTLE_CASES];
    u8 recentCursor;
} FairAIState;

_Static_assert(sizeof(FairAIState) <= 768, "trainer AI battle-owned state exceeds its heap budget");
_Static_assert(AI_CFG_PERSISTENT_PREDICTION_CASES == AI_PREDICTION_CASE_COUNT,
    "trainer AI tuning and save prediction counts disagree");

static FairAIState *sFairAIStatePtr;
#define sFairAIState (*sFairAIStatePtr)

static const u16 sFairAIProtectSuccessDenominators[PROTECT_SUCCESS_DENOMINATOR_COUNT] = PROTECT_SUCCESS_DENOMINATORS;

static FairAIKnownMon *FairAI_GetRecord(struct BattleStruct *ctx, int battlerId)
{
    u8 slot = ctx->sel_mons_no[battlerId];

    return slot < FAIR_AI_PARTY_SIZE ? &sFairAIState.playerMons[slot] : NULL;
}

static void FairAI_AddKnownMove(FairAIKnownMon *known, u16 move)
{
    int index;

    if (move == MOVE_NONE || move > NUM_OF_MOVES || known->moveCount >= FAIR_AI_PREDICTED_MOVES) {
        return;
    }
    index = known->moveCount++;
    known->moves[index] = move;
}

static void FairAI_LoadExactPartyMon(struct PartyPokemon *mon, FairAIKnownMon *known)
{
    int i;

    memset(known, 0, sizeof(*known));
    known->species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    known->form = GetMonData(mon, MON_DATA_FORM, NULL);
    known->ability = GetMonData(mon, MON_DATA_ABILITY, NULL);
    known->item = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
    known->hp = GetMonData(mon, MON_DATA_HP, NULL);
    known->maxHp = GetMonData(mon, MON_DATA_MAXHP, NULL);
    known->attack = GetMonData(mon, MON_DATA_ATTACK, NULL);
    known->defense = GetMonData(mon, MON_DATA_DEFENSE, NULL);
    known->speed = GetMonData(mon, MON_DATA_SPEED, NULL);
    known->spAttack = GetMonData(mon, MON_DATA_SPECIAL_ATTACK, NULL);
    known->spDefense = GetMonData(mon, MON_DATA_SPECIAL_DEFENSE, NULL);
    known->level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    known->condition = GetMonData(mon, MON_DATA_STATUS, NULL);
    known->type1 = PokeFormNoPersonalParaGet(known->species, known->form, PERSONAL_TYPE_1);
    known->type2 = PokeFormNoPersonalParaGet(known->species, known->form, PERSONAL_TYPE_2);
    known->hpPercent = known->maxHp == 0 ? 0 : known->hp * 100 / known->maxHp;
    known->moveCount = 0;
    for (i = 0; i < 8; i++) {
        known->stages[i] = 6;
    }
    for (i = 0; i < MAX_MON_MOVES; i++) {
        u16 move = GetMonData(mon, MON_DATA_MOVE1 + i, NULL);
        known->pp[i] = GetMonData(mon, MON_DATA_MOVE1PP + i, NULL);
        if (move != MOVE_NONE && known->pp[i] != 0) {
            FairAI_AddKnownMove(known, move);
        }
    }
}

static void FairAI_RecordActive(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId)
{
    FairAIKnownMon *known;
    struct BattlePokemon *mon;
    int i;

    if (!ClientBelongsToPlayer(bsys, battlerId) || !IsBattlerSlotValid(bsys, battlerId)) {
        return;
    }
    known = FairAI_GetRecord(ctx, battlerId);
    mon = &ctx->battlemon[battlerId];
    if (known == NULL || mon->species == SPECIES_NONE) {
        return;
    }
    known->species = mon->species;
    known->form = mon->form_no;
    known->ability = mon->ability;
    known->item = mon->item;
    known->hp = mon->hp;
    known->maxHp = mon->maxhp;
    known->attack = mon->attack;
    known->defense = mon->defense;
    known->speed = mon->speed;
    known->spAttack = mon->spatk;
    known->spDefense = mon->spdef;
    known->type1 = mon->type1;
    known->type2 = mon->type2;
    known->level = mon->level;
    known->hpPercent = mon->maxhp == 0 ? 0 : mon->hp * 100 / mon->maxhp;
    known->condition = mon->condition;
    known->condition2 = mon->condition2;
    known->effectFlags = mon->effect_of_moves;
    known->semanticState = mon->moveeffect.flashFire ? FAIR_AI_MON_STATE_FLASH_FIRE : 0;
    memcpy(known->stages, mon->states, sizeof(known->stages));
    known->moveCount = 0;
    for (i = 0; i < MAX_MON_MOVES; i++) {
        known->pp[i] = mon->pp[i];
        if (mon->move[i] != MOVE_NONE && mon->pp[i] != 0 && !(ctx->moveTbl[mon->move[i]].flag & FLAG_UNUSED_MOVE)) {
            FairAI_AddKnownMove(known, mon->move[i]);
        }
    }
}

static void FairAI_RefreshKnowledge(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId)
{
    int slot;

    memset(sFairAIState.playerMons, 0, sizeof(sFairAIState.playerMons));
    for (slot = 0; slot < BattleWorkPokeCountGet(bsys, battlerId) && slot < FAIR_AI_PARTY_SIZE; slot++) {
        struct PartyPokemon *partyMon = BattleWorkPokemonParamGet(bsys, battlerId, slot);
        if (partyMon != NULL) {
            FairAI_LoadExactPartyMon(partyMon, &sFairAIState.playerMons[slot]);
        }
    }
    FairAI_RecordActive(bsys, ctx, battlerId);
}

BOOL TrainerAI_UsesStrategicLayer(u32 aiFlags)
{
    return (aiFlags & F_TRAINER_EXPERT_AI) == F_TRAINER_EXPERT_AI;
}

static BOOL FairAI_IsSupportedBattle(struct BattleSystem *bsys, int battlerId)
{
    u32 type = BattleTypeGet(bsys);
    u32 unsupported = BATTLE_TYPE_DOUBLES | BATTLE_TYPE_LINK | BATTLE_TYPE_MULTI | BATTLE_TYPE_TAG
        | BATTLE_TYPE_SAFARI | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_PAL_PARK | BATTLE_TYPE_TUTORIAL
        | BATTLE_TYPE_BUG_CONTEST;

    return (type & BATTLE_TYPE_TRAINER) != 0 && (type & unsupported) == 0
        && IsClientEnemy(bsys, battlerId)
        && TrainerAI_UsesStrategicLayer(bsys->trainers[battlerId].aibit);
}

void FairTrainerAI_Reset(struct BattleSystem *bsys)
{
    sFairAIStatePtr = sys_AllocMemory(HEAPID_BATTLE_HEAP, sizeof(*sFairAIStatePtr));
    if (sFairAIStatePtr == NULL) {
        return;
    }
    memset(&sFairAIState, 0, sizeof(sFairAIState));
    (void)bsys;
}

static u8 FairAI_SituationClass(const FairAIMonModel *trainer, const FairAIKnownMon *player)
{
    int trainerHp = trainer->maxHp == 0 ? 0 : trainer->hp * 100 / trainer->maxHp;
    int playerHp = player == NULL ? 0 : player->hpPercent;

    if (trainerHp <= 25) {
        return 3;
    }
    if (trainerHp + 20 < playerHp) {
        return 2;
    }
    if (trainerHp > playerHp + 20) {
        return 0;
    }
    return 1;
}

static void FairAI_RecordCommitmentResult(FairAICommitment *commitment, BOOL success)
{
    AIPredictionMemory *memory = &Sav2_Misc_get(SaveBlock2_get())->trainerAIMemory;
    AIPredictionCase *recent = &sFairAIState.recentCases[sFairAIState.recentCursor];
    AIPredictionCase *entry = NULL;
    int i;

    if (memory->version != 1) {
        memset(memory, 0, sizeof(*memory));
        memory->version = 1;
    }
    for (i = 0; i < memory->count; i++) {
        if (memory->cases[i].situation == commitment->situation
            && memory->cases[i].actionClass == commitment->actionClass) {
            entry = &memory->cases[i];
            break;
        }
    }
    if (entry == NULL) {
        entry = &memory->cases[memory->cursor];
        memset(entry, 0, sizeof(*entry));
        entry->situation = commitment->situation;
        entry->actionClass = commitment->actionClass;
        memory->cursor = (memory->cursor + 1) % AI_PREDICTION_CASE_COUNT;
        if (memory->count < AI_PREDICTION_CASE_COUNT) {
            memory->count++;
        }
    }
    if (entry->observations != 0xFF) {
        entry->observations++;
    }
    if (success && entry->chosenCount != 0xFF) {
        entry->chosenCount++;
    }
    entry->confidence = entry->observations == 0 ? 0 : entry->chosenCount * 255 / entry->observations;
    entry->recency = 0;
    if (success && memory->readBalance < 8) {
        memory->readBalance++;
    } else if (!success && memory->readBalance > -8) {
        memory->readBalance--;
    }
    *recent = *entry;
    sFairAIState.recentCursor = (sFairAIState.recentCursor + 1) % AI_CFG_RECENT_BATTLE_CASES;
}

static void FairAI_ResolveCommitment(struct BattleSystem *bsys, int battlerId)
{
    FairAICommitment *commitment = &sFairAIState.commitments[battlerId];
    if (!commitment->valid) {
        return;
    }
    if (bsys->sp->total_turn <= commitment->turn
        && bsys->sp->sel_mons_no[commitment->playerBattler] == commitment->playerSlot) {
        return;
    }
    if (IsBattlerSlotValid(bsys, commitment->playerBattler)) {
        FairAI_RecordCommitmentResult(commitment,
            bsys->sp->sel_mons_no[commitment->playerBattler] != commitment->playerSlot);
    }
    commitment->valid = FALSE;
}

void FairTrainerAI_BattleEnd(struct BattleSystem *bsys)
{
    int battlerId;
    if (sFairAIStatePtr == NULL) {
        return;
    }
    for (battlerId = 0; battlerId < CLIENT_MAX; battlerId++) {
        sFairAIState.commitments[battlerId].valid = FALSE;
    }
    (void)bsys;
    sFairAIStatePtr = NULL;
}

static int FairAI_GetTarget(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId)
{
    int target = BATTLER_OPPONENT(battlerId);

    if (!IsBattlerSlotValid(bsys, target) || ctx->battlemon[target].hp == 0) {
        target = BATTLER_ACROSS(battlerId);
    }
    return target;
}

static int FairAI_EstimateStat(const FairAIKnownMon *mon, int field)
{
    switch (field) {
    case PERSONAL_BASE_HP:
        return mon->maxHp;
    case PERSONAL_BASE_ATTACK:
        return mon->attack;
    case PERSONAL_BASE_DEFENSE:
        return mon->defense;
    case PERSONAL_BASE_SPEED:
        return mon->speed;
    case PERSONAL_BASE_SP_ATTACK:
        return mon->spAttack;
    case PERSONAL_BASE_SP_DEFENSE:
        return mon->spDefense;
    default:
        return 1;
    }
}

static int FairAI_ApplyStage(int stat, int stage)
{
    if (stage > 12) {
        stage = 12;
    } else if (stage < 0) {
        stage = 0;
    }
    return stat * StatBoostModifiers[stage][0] / StatBoostModifiers[stage][1];
}

static u8 FairAI_GetEffectClass(u16 effect)
{
    int i;

    if (effect >= AI_EFFECT_ID_LIMIT) {
        return AI_EFFECT_CLASS_NONE;
    }
    for (i = 0; i < AI_EFFECT_CLASS_ENTRY_COUNT; i++) {
        u16 entry = sAIEffectClassEntries[i];
        if ((entry & AI_EFFECT_ID_MASK) == effect) {
            return entry >> AI_EFFECT_CLASS_SHIFT;
        }
    }
    return AI_EFFECT_CLASS_NONE;
}

static u8 FairAI_GetMoveClass(u16 move)
{
    int i;

    for (i = 0; i < AI_MOVE_CLASS_ENTRY_COUNT; i++) {
        u16 entry = sAIMoveClasses[i];
        if ((entry & AI_MOVE_CLASS_ID_MASK) == move) {
            return entry >> AI_MOVE_CLASS_SHIFT;
        }
    }
    return AI_MOVE_CLASS_NONE;
}

static u8 FairAI_GetAbilityTypeBehavior(u16 ability, u8 type)
{
    int i;

    for (i = 0; i < AI_ABILITY_TYPE_RULE_COUNT; i++) {
        u16 entry = sAIAbilityTypeRules[i];
        if ((entry & AI_ABILITY_ID_MASK) == ability
            && ((entry >> AI_ABILITY_TYPE_SHIFT) & AI_ABILITY_TYPE_MASK) == type) {
            return (entry >> AI_ABILITY_TYPE_BEHAVIOR_SHIFT) + 1;
        }
    }
    return AI_ABILITY_TYPE_NONE;
}

static u16 FairAI_GetMoveTransition(u16 move)
{
    int i;

    for (i = 0; i < AI_MOVE_TRANSITION_COUNT; i++) {
        if ((sAIMoveTransitions[i] & AI_MOVE_TRANSITION_ID_MASK) == move) {
            return sAIMoveTransitions[i];
        }
    }
    return 0;
}

static void FairAI_LoadMoveState(struct BattleStruct *ctx, int battlerId, FairAIMoveState *state)
{
    u16 transition;
    u8 transitionKind;

    memset(state, 0, sizeof(*state));
    state->previousMove = ctx->waza_no_old[battlerId];
    transition = FairAI_GetMoveTransition(state->previousMove);
    transitionKind = (transition >> AI_MOVE_TRANSITION_KIND_SHIFT) & AI_MOVE_TRANSITION_KIND_MASK;
    if (transitionKind == AI_MOVE_TRANSITION_REPEAT_SCALING) {
        u8 count = ctx->battlemon[battlerId].moveeffect.furyCutterCount;
        state->repeatProgress = count == 0 ? 0 : count - 1;
    }
    if (!(ctx->battlemon[battlerId].condition2 & STATUS2_LOCKED_INTO_MOVE)) {
        return;
    }
    state->forcedMove = state->previousMove;
    state->flags |= FAIR_AI_MOVE_STATE_FORCED;
    if (transitionKind == AI_MOVE_TRANSITION_FORCE_REPEAT_SCALING) {
        u8 remaining = ctx->battlemon[battlerId].moveeffect.rolloutCount;
        state->forcedProgress = remaining < 4 ? 4 - remaining : 0;
    }
}

static int FairAI_MovePowerAtState(struct BattleStruct *ctx, u16 move, const FairAIMoveState *state)
{
    u16 transition = FairAI_GetMoveTransition(move);
    u8 transitionKind = (transition >> AI_MOVE_TRANSITION_KIND_SHIFT) & AI_MOVE_TRANSITION_KIND_MASK;
    u8 maximumProgress = transition >> AI_MOVE_TRANSITION_PROGRESS_SHIFT;
    int progress = 0;
    int power = ctx->moveTbl[move].power;

    if (transition == 0 || state == NULL) {
        return power;
    }
    if (transitionKind == AI_MOVE_TRANSITION_FORCE_REPEAT_SCALING
        && state->forcedMove == move) {
        progress = state->forcedProgress;
    } else if (transitionKind == AI_MOVE_TRANSITION_REPEAT_SCALING
        && state->previousMove == move) {
        progress = state->repeatProgress;
    }
    if (progress >= maximumProgress) {
        progress = maximumProgress - 1;
    }
    while (progress-- > 0) {
        power *= 2;
    }
    return power;
}

static void FairAI_AdvanceMoveStateOnHit(u16 move, const FairAIMoveState *before,
    FairAIMoveState *after)
{
    u16 transition = FairAI_GetMoveTransition(move);
    u8 transitionKind = (transition >> AI_MOVE_TRANSITION_KIND_SHIFT) & AI_MOVE_TRANSITION_KIND_MASK;
    u8 maximumProgress = transition >> AI_MOVE_TRANSITION_PROGRESS_SHIFT;

    *after = *before;
    after->previousMove = move;
    if (transition == 0) {
        after->flags &= ~FAIR_AI_MOVE_STATE_FORCED;
        after->forcedMove = MOVE_NONE;
        after->repeatProgress = 0;
        return;
    }
    if (transitionKind == AI_MOVE_TRANSITION_FORCE_REPEAT_SCALING) {
        after->flags |= FAIR_AI_MOVE_STATE_FORCED;
        after->forcedMove = move;
        after->forcedProgress = before->forcedMove == move ? before->forcedProgress + 1 : 1;
        if (after->forcedProgress >= maximumProgress) {
            after->flags &= ~FAIR_AI_MOVE_STATE_FORCED;
            after->forcedMove = MOVE_NONE;
        }
    } else if (transitionKind == AI_MOVE_TRANSITION_REPEAT_SCALING) {
        after->repeatProgress = before->previousMove == move ? before->repeatProgress + 1 : 1;
        if (after->repeatProgress >= maximumProgress) {
            after->repeatProgress = maximumProgress - 1;
        }
    }
}

static int FairAI_TypeFactorOne(u8 moveType, u8 defenderType)
{
    int i;

    for (i = 0; TypeEffectivenessTable[i][0] != TYPE_ENDTABLE; i++) {
        if (TypeEffectivenessTable[i][0] == moveType && TypeEffectivenessTable[i][1] == defenderType) {
            return TypeEffectivenessTable[i][2] * 10;
        }
    }
    return 100;
}

static int FairAI_AbilityTypeFactor(u8 moveType, u16 ability)
{
    u8 behavior = FairAI_GetAbilityTypeBehavior(ability, moveType);

    if (behavior >= AI_ABILITY_TYPE_IMMUNE && behavior <= AI_ABILITY_TYPE_ABSORBBOOST) {
        return 0;
    }
    if (behavior == AI_ABILITY_TYPE_MULTIPLY50) {
        return 50;
    }
    if (behavior == AI_ABILITY_TYPE_MULTIPLY125) {
        return 125;
    }
    return 100;
}

static int FairAI_TypeFactor(u8 moveType, u8 type1, u8 type2, u16 ability)
{
    int factor = FairAI_TypeFactorOne(moveType, type1);

    if (type2 != type1) {
        factor = factor * FairAI_TypeFactorOne(moveType, type2) / 100;
    }
    return factor * FairAI_AbilityTypeFactor(moveType, ability) / 100;
}

static void FairAI_FillActiveModel(struct BattleStruct *ctx, int battlerId, FairAIMonModel *model)
{
    struct BattlePokemon *mon = &ctx->battlemon[battlerId];
    int i;

    memset(model, 0, sizeof(*model));
    model->species = mon->species;
    model->ability = mon->ability;
    model->hp = mon->hp;
    model->maxHp = mon->maxhp;
    model->attack = mon->attack;
    model->defense = mon->defense;
    model->speed = mon->speed;
    model->spAttack = mon->spatk;
    model->spDefense = mon->spdef;
    model->type1 = mon->type1;
    model->type2 = mon->type2;
    model->form = mon->form_no;
    model->level = mon->level;
    model->condition = mon->condition;
    model->condition2 = mon->condition2;
    model->effectFlags = mon->effect_of_moves;
    if (mon->moveeffect.flashFire) {
        model->semanticState |= FAIR_AI_MON_STATE_FLASH_FIRE;
    }
    for (i = 0; i < MAX_MON_MOVES; i++) {
        model->moves[i] = mon->move[i];
        model->pp[i] = mon->pp[i];
    }
    memcpy(model->stages, mon->states, sizeof(model->stages));
}

static void FairAI_FillReserveModel(struct PartyPokemon *mon, FairAIMonModel *model)
{
    int i;

    memset(model, 0, sizeof(*model));
    model->species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    model->form = GetMonData(mon, MON_DATA_FORM, NULL);
    model->ability = GetMonData(mon, MON_DATA_ABILITY, NULL);
    model->hp = GetMonData(mon, MON_DATA_HP, NULL);
    model->maxHp = GetMonData(mon, MON_DATA_MAXHP, NULL);
    model->attack = GetMonData(mon, MON_DATA_ATTACK, NULL);
    model->defense = GetMonData(mon, MON_DATA_DEFENSE, NULL);
    model->speed = GetMonData(mon, MON_DATA_SPEED, NULL);
    model->spAttack = GetMonData(mon, MON_DATA_SPECIAL_ATTACK, NULL);
    model->spDefense = GetMonData(mon, MON_DATA_SPECIAL_DEFENSE, NULL);
    model->level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    model->condition = GetMonData(mon, MON_DATA_STATUS, NULL);
    model->type1 = PokeFormNoPersonalParaGet(model->species, model->form, PERSONAL_TYPE_1);
    model->type2 = PokeFormNoPersonalParaGet(model->species, model->form, PERSONAL_TYPE_2);
    for (i = 0; i < 8; i++) {
        model->stages[i] = 6;
    }
    for (i = 0; i < MAX_MON_MOVES; i++) {
        model->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, NULL);
        model->pp[i] = GetMonData(mon, MON_DATA_MOVE1PP + i, NULL);
    }
}

static int FairAI_FixedDamage(const FairAIMonModel *attacker, const FairAIKnownMon *target, u16 move)
{
    int targetHp = FairAI_EstimateStat(target, PERSONAL_BASE_HP);

    switch (move) {
    case MOVE_SONIC_BOOM:
        return 2000 / targetHp;
    case MOVE_DRAGON_RAGE:
        return 4000 / targetHp;
    case MOVE_SEISMIC_TOSS:
    case MOVE_NIGHT_SHADE:
    case MOVE_PSYWAVE:
        return attacker->level * 100 / targetHp;
    case MOVE_SUPER_FANG:
        return target->hp * 50 / (targetHp ? targetHp : 1);
    case MOVE_FINAL_GAMBIT:
        return attacker->hp * 100 / targetHp;
    case MOVE_GUILLOTINE:
    case MOVE_HORN_DRILL:
    case MOVE_FISSURE:
    case MOVE_SHEER_COLD:
        return target->hp * 30 / (targetHp ? targetHp : 1);
    default:
        return 0;
    }
}

static int FairAI_Damage(struct BattleStruct *ctx, const FairAIMonModel *attacker,
    const FairAIKnownMon *target, u16 move)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    int attack;
    int defense;
    int targetHp;
    int damage;
    int typeFactor;
    int accuracy;
    u8 moveType;

    if (attacker == NULL || target == NULL || attacker->species == SPECIES_NONE
        || target->species == SPECIES_NONE || data->split == SPLIT_STATUS) {
        return 0;
    }
    if (data->power == 0) {
        return FairAI_FixedDamage(attacker, target, move);
    }
    moveType = GetAdjustedMoveTypeBasics(ctx, move, attacker->ability, data->type);
    typeFactor = FairAI_TypeFactor(moveType, target->type1, target->type2, target->ability);
    if (typeFactor == 0) {
        return 0;
    }
    if (data->split == SPLIT_PHYSICAL) {
        attack = FairAI_ApplyStage(attacker->attack, attacker->stages[STAT_ATTACK]);
        defense = FairAI_ApplyStage(target->defense, target->stages[STAT_DEFENSE]);
    } else {
        attack = FairAI_ApplyStage(attacker->spAttack, attacker->stages[STAT_SPECIAL_ATTACK]);
        defense = FairAI_ApplyStage(target->spDefense, target->stages[STAT_SPECIAL_DEFENSE]);
    }
    targetHp = FairAI_EstimateStat(target, PERSONAL_BASE_HP);
    damage = (((2 * attacker->level / 5 + 2) * data->power * attack / (defense ? defense : 1)) / 50) + 2;
    damage = damage * typeFactor / 100;
    if (moveType == attacker->type1 || moveType == attacker->type2) {
        damage = damage * 3 / 2;
    }
    if (moveType == TYPE_FIRE && (attacker->semanticState & FAIR_AI_MON_STATE_FLASH_FIRE)) {
        damage = damage * 3 / 2;
    }
    accuracy = data->accuracy == 0 ? 100 : data->accuracy;
    return damage * 100 / targetHp * accuracy / 100;
}

static int FairAI_DamageToTrainerAtState(struct BattleStruct *ctx, const FairAIKnownMon *attacker,
    const FairAIMonModel *target, u16 move, const FairAIMoveState *state)
{
    int basePower = ctx->moveTbl[move].power;
    int power = FairAI_MovePowerAtState(ctx, move, state);
    int damage = FairAI_Damage(ctx, attacker, target, move);

    if (basePower == 0 || power == basePower) {
        return damage;
    }
    // The ordinary approximation has a small fixed +2 term. Scaling its final
    // result is intentionally conservative and avoids duplicating the damage
    // formula solely for consecutive-use moves.
    return damage * power / basePower;
}

static int FairAI_Threat(struct BattleStruct *ctx, const FairAIKnownMon *attacker, const FairAIMonModel *target, int split)
{
    int best = 0;
    int i;

    if (attacker == NULL) {
        return 0;
    }
    for (i = 0; i < attacker->moveCount; i++) {
        u16 move = attacker->moves[i];
        int damage;

        if (ctx->moveTbl[move].split == SPLIT_STATUS || (split >= 0 && ctx->moveTbl[move].split != split)) {
            continue;
        }
        damage = FairAI_Damage(ctx, attacker, target, move);
        if (damage > best) {
            best = damage;
        }
    }
    return best;
}

static int FairAI_DamageGapWeight(int gap)
{
    return gap <= 4 ? 128 : gap <= 10 ? 64 : gap <= 20 ? 24 : gap <= 35 ? 8 : 1;
}

static int FairAI_ExpectedThreat(struct BattleStruct *ctx, const FairAIKnownMon *attacker, const FairAIMonModel *target)
{
    int damage[MAX_MON_MOVES];
    int best = 0;
    int total = 0;
    int totalWeight = 0;
    int i;

    if (attacker == NULL || attacker->moveCount == 0) {
        return 0;
    }
    for (i = 0; i < attacker->moveCount; i++) {
        damage[i] = FairAI_Damage(ctx, attacker, target, attacker->moves[i]);
        if (damage[i] > best) {
            best = damage[i];
        }
    }
    for (i = 0; i < attacker->moveCount; i++) {
        int gap = best - damage[i];
        int weight = FairAI_DamageGapWeight(gap);
        total += damage[i] * weight;
        totalWeight += weight;
    }
    return totalWeight == 0 ? 0 : total / totalWeight;
}

static int FairAI_PredictedDamageIntoSwitch(struct BattleStruct *ctx, const FairAIKnownMon *player,
    const FairAIMonModel *currentTrainer, const FairAIMonModel *reserve)
{
    int currentDamage[MAX_MON_MOVES];
    int reserveDamage[MAX_MON_MOVES];
    int bestCurrent = 0;
    int total = 0;
    int totalWeight = 0;
    int i;

    if (player == NULL || currentTrainer == NULL) {
        return 0;
    }
    for (i = 0; i < player->moveCount; i++) {
        currentDamage[i] = FairAI_Damage(ctx, player, currentTrainer, player->moves[i]);
        reserveDamage[i] = FairAI_Damage(ctx, player, reserve, player->moves[i]);
        if (currentDamage[i] > bestCurrent) {
            bestCurrent = currentDamage[i];
        }
    }
    for (i = 0; i < player->moveCount; i++) {
        int weight = FairAI_DamageGapWeight(bestCurrent - currentDamage[i]);
        total += reserveDamage[i] * weight;
        totalWeight += weight;
    }
    return totalWeight == 0 ? 0 : total / totalWeight;
}

static int FairAI_EffectivePriority(struct BattleStruct *ctx, u16 move, u16 ability, u16 hp, u16 maxHp)
{
    int priority = ctx->moveTbl[move].priority;
    u8 effectClass = FairAI_GetEffectClass(ctx->moveTbl[move].effect);

    if (ability == ABILITY_PRANKSTER && ctx->moveTbl[move].split == SPLIT_STATUS) {
        priority++;
    }
    if (ability == ABILITY_GALE_WINGS && ctx->moveTbl[move].type == TYPE_FLYING && hp == maxHp) {
        priority++;
    }
    if (ability == ABILITY_TRIAGE
        && (effectClass == AI_EFFECT_CLASS_RECOVERHALF || effectClass == AI_EFFECT_CLASS_LEECHSEED)) {
        priority += 3;
    }
    return priority;
}

static int FairAI_ActsFirstChance(struct BattleStruct *ctx, const FairAIMonModel *trainer, u16 trainerMove, const FairAIKnownMon *player, u16 playerMove)
{
    int trainerPriority = FairAI_EffectivePriority(ctx, trainerMove, trainer->ability, trainer->hp, trainer->maxHp);
    int playerPriority = FairAI_EffectivePriority(ctx, playerMove, player->ability, player->hp, player->maxHp);
    int trainerSpeed;
    int playerSpeed;

    if (trainerPriority != playerPriority) {
        return trainerPriority > playerPriority ? 100 : 0;
    }
    trainerSpeed = FairAI_ApplyStage(trainer->speed, trainer->stages[STAT_SPEED]);
    playerSpeed = FairAI_ApplyStage(player->speed, player->stages[STAT_SPEED]);
    if (ctx->field_condition & FIELD_CONDITION_TRICK_ROOM) {
        return trainerSpeed < playerSpeed ? 100 : trainerSpeed > playerSpeed ? 0 : 50;
    }
    return trainerSpeed > playerSpeed ? 100 : trainerSpeed < playerSpeed ? 0 : 50;
}

static int FairAI_BestDamage(struct BattleStruct *ctx, const FairAIMonModel *attacker, const FairAIKnownMon *target, int split)
{
    int best = 0;
    int i;

    for (i = 0; i < MAX_MON_MOVES; i++) {
        int damage;
        u16 move = attacker->moves[i];

        if (move == MOVE_NONE || attacker->pp[i] == 0 || ctx->moveTbl[move].split == SPLIT_STATUS
            || (split >= 0 && ctx->moveTbl[move].split != split)) {
            continue;
        }
        damage = FairAI_Damage(ctx, attacker, target, move);
        if (damage > best) {
            best = damage;
        }
    }
    return best;
}

static BOOL FairAI_GetStageChange(u16 effect, int *stat, int *change, BOOL *targetsSelf)
{
    *targetsSelf = TRUE;
    if (effect >= MOVE_EFFECT_ATK_UP && effect <= MOVE_EFFECT_EVA_UP) {
        *stat = effect - MOVE_EFFECT_ATK_UP + STAT_ATTACK;
        *change = 1;
        return TRUE;
    }
    if (effect >= MOVE_EFFECT_ATK_UP_2 && effect <= MOVE_EFFECT_EVA_UP_2) {
        *stat = effect - MOVE_EFFECT_ATK_UP_2 + STAT_ATTACK;
        *change = 2;
        return TRUE;
    }
    if (effect >= MOVE_EFFECT_ATK_UP_3 && effect <= MOVE_EFFECT_EVA_UP_3) {
        *stat = effect - MOVE_EFFECT_ATK_UP_3 + STAT_ATTACK;
        *change = 3;
        return TRUE;
    }
    *targetsSelf = FALSE;
    if (effect >= MOVE_EFFECT_ATK_DOWN && effect <= MOVE_EFFECT_EVA_DOWN) {
        *stat = effect - MOVE_EFFECT_ATK_DOWN + STAT_ATTACK;
        *change = -1;
        return TRUE;
    }
    if (effect >= MOVE_EFFECT_ATK_DOWN_2 && effect <= MOVE_EFFECT_EVA_DOWN_2) {
        *stat = effect - MOVE_EFFECT_ATK_DOWN_2 + STAT_ATTACK;
        *change = -2;
        return TRUE;
    }
    if (effect >= MOVE_EFFECT_ATK_DOWN_3 && effect <= MOVE_EFFECT_EVA_DOWN_3) {
        *stat = effect - MOVE_EFFECT_ATK_DOWN_3 + STAT_ATTACK;
        *change = -3;
        return TRUE;
    }
    return FALSE;
}

static int FairAI_StageUtility(struct BattleStruct *ctx, FairAIMonModel *trainer, const FairAIKnownMon *target, int targetBattler, int stat, int change, BOOL targetsSelf, int incoming)
{
    int before;
    int after;
    int stage;

    if (targetsSelf) {
        stage = trainer->stages[stat];
        if ((change > 0 && stage >= 12) || (change < 0 && stage <= 0)) {
            return FAIR_AI_INVALID;
        }
        if (stat == STAT_ATTACK || stat == STAT_SPECIAL_ATTACK) {
            int hpPercent = trainer->maxHp == 0 ? 0 : trainer->hp * 100 / trainer->maxHp;
            int bestUtility = FAIR_AI_INVALID;
            int turnsBefore;
            int setupUses;
            before = FairAI_BestDamage(ctx, trainer, target, stat == STAT_ATTACK ? SPLIT_PHYSICAL : SPLIT_SPECIAL);
            if (before <= 0 || incoming >= hpPercent) {
                return FAIR_AI_INVALID;
            }
            turnsBefore = (target->hpPercent + before - 1) / before;
            if (turnsBefore > AI_CFG_PLAN_TURN_CAP) {
                turnsBefore = AI_CFG_PLAN_TURN_CAP;
            }
            for (setupUses = 1; setupUses < AI_CFG_PLAN_TURN_CAP; setupUses++) {
                int boostedStage = stage + change * setupUses;
                int turnsAfter;
                int utility;

                if (boostedStage > 12) {
                    boostedStage = 12;
                }
                after = before * StatBoostModifiers[boostedStage][0]
                    * StatBoostModifiers[stage][1]
                    / (StatBoostModifiers[boostedStage][1] * StatBoostModifiers[stage][0]);
                if (after <= 0 || incoming * setupUses >= hpPercent) {
                    break;
                }
                turnsAfter = setupUses + (target->hpPercent + after - 1) / after;
                if (turnsAfter > AI_CFG_PLAN_TURN_CAP) {
                    turnsAfter = AI_CFG_PLAN_TURN_CAP;
                }
                if (incoming * (turnsAfter - 1) >= hpPercent) {
                    continue;
                }
                utility = (turnsBefore - turnsAfter) * incoming
                    + (after - before) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
                if (utility > bestUtility) {
                    bestUtility = utility;
                }
                if (boostedStage == 12) {
                    break;
                }
            }
            return bestUtility;
        }
        if (stat == STAT_DEFENSE || stat == STAT_SPECIAL_DEFENSE) {
            before = incoming;
            after = before * StatBoostModifiers[stage][0]
                * StatBoostModifiers[stage + change > 12 ? 12 : stage + change][1]
                / (StatBoostModifiers[stage][1] * StatBoostModifiers[stage + change > 12 ? 12 : stage + change][0]);
            return (before - after) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
        }
        if (stat == STAT_SPEED) {
            int targetSpeed = FairAI_ApplyStage(target->speed, target->stages[STAT_SPEED]);
            int oldSpeed = FairAI_ApplyStage(trainer->speed, stage);
            int newSpeed = FairAI_ApplyStage(trainer->speed, stage + change);
            BOOL trickRoom = (ctx->field_condition & FIELD_CONDITION_TRICK_ROOM) != 0;
            BOOL oldFirst = trickRoom ? oldSpeed < targetSpeed : oldSpeed > targetSpeed;
            BOOL newFirst = trickRoom ? newSpeed < targetSpeed : newSpeed > targetSpeed;

            if (oldFirst == newFirst) {
                return FAIR_AI_INVALID;
            }
            return newFirst ? incoming / 2 - incoming : -incoming - incoming / 2;
        }
        return incoming / 4 - incoming;
    }
    if (targetBattler < 0) {
        return -incoming;
    }
    stage = ctx->battlemon[targetBattler].states[stat];
    if ((change < 0 && stage <= 0) || (change > 0 && stage >= 12)) {
        return FAIR_AI_INVALID;
    }
    if (stat == STAT_ATTACK) {
        before = FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL);
    } else if (stat == STAT_SPECIAL_ATTACK) {
        before = FairAI_Threat(ctx, target, trainer, SPLIT_SPECIAL);
    } else if (stat == STAT_DEFENSE) {
        before = FairAI_BestDamage(ctx, trainer, target, SPLIT_PHYSICAL);
    } else if (stat == STAT_SPECIAL_DEFENSE) {
        before = FairAI_BestDamage(ctx, trainer, target, SPLIT_SPECIAL);
    } else if (stat == STAT_SPEED) {
        int trainerSpeed = FairAI_ApplyStage(trainer->speed, trainer->stages[STAT_SPEED]);
        int targetSpeed = FairAI_ApplyStage(target->speed, stage);
        int loweredSpeed = FairAI_ApplyStage(target->speed, stage + change < 0 ? 0 : stage + change);
        if (trainerSpeed > targetSpeed || trainerSpeed <= loweredSpeed) {
            return FAIR_AI_INVALID;
        }
        return incoming / 2;
    } else {
        before = incoming / 4;
    }
    after = before * StatBoostModifiers[stage + change < 0 ? 0 : stage + change][0]
        * StatBoostModifiers[stage][1]
        / (StatBoostModifiers[stage + change < 0 ? 0 : stage + change][1] * StatBoostModifiers[stage][0]);
    if (stat == STAT_DEFENSE || stat == STAT_SPECIAL_DEFENSE) {
        return (after - before) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
    }
    return (before - after) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
}

static int FairAI_MergeSetupUtility(int total, int value, int incoming)
{
    if (value == FAIR_AI_INVALID) {
        return total;
    }
    return total == FAIR_AI_INVALID ? value : total + value + incoming;
}

static int FairAI_CombinedSelfUtility(struct BattleStruct *ctx, FairAIMonModel *trainer,
    const FairAIKnownMon *target, int incoming, int stat1, int stat2)
{
    int first = FairAI_StageUtility(ctx, trainer, target, -1, stat1, 1, TRUE, incoming);
    int second = FairAI_StageUtility(ctx, trainer, target, -1, stat2, 1, TRUE, incoming);

    return FairAI_MergeSetupUtility(first, second, incoming);
}

static int FairAI_ProtectSuccessDenominator(struct BattleStruct *ctx, int battlerId)
{
    int count;

    count = ctx->protectSuccessTurns[battlerId];
    if (count >= PROTECT_SUCCESS_DENOMINATOR_COUNT) {
        count = PROTECT_SUCCESS_DENOMINATOR_COUNT - 1;
    }
    return sFairAIProtectSuccessDenominators[count];
}

static int FairAI_CurseUtility(struct BattleSystem *bsys, struct BattleStruct *ctx,
    FairAIMonModel *trainer, const FairAIKnownMon *target, int targetBattler, int incoming)
{
    int total;

    if (trainer->type1 == TYPE_GHOST || trainer->type2 == TYPE_GHOST) {
        int persistence;
        int hpPercent = trainer->maxHp == 0 ? 0 : trainer->hp * 100 / trainer->maxHp;
        int cost = hpPercent <= 50 ? hpPercent + AI_CFG_TEAM_ASSET_WEIGHT : 50;

        if ((target->condition2 & (STATUS2_CURSE | STATUS2_SUBSTITUTE)) != 0) {
            return FAIR_AI_INVALID;
        }
        persistence = targetBattler < 0 || CanSwitchMon(bsys, ctx, targetBattler)
            ? AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT : 100;
        return target->hpPercent * FAIR_AI_FUTURE_PERCENT / 100 * persistence / 100
            - cost - incoming;
    }

    total = FairAI_StageUtility(ctx, trainer, target, -1, STAT_ATTACK, 1, TRUE, incoming);
    total = FairAI_MergeSetupUtility(total,
        FairAI_StageUtility(ctx, trainer, target, -1, STAT_DEFENSE, 1, TRUE, incoming), incoming);
    return FairAI_MergeSetupUtility(total,
        FairAI_StageUtility(ctx, trainer, target, -1, STAT_SPEED, -1, TRUE, incoming), incoming);
}

static int FairAI_CurePartyStatusUtility(struct BattleSystem *bsys, int battlerId, int incoming)
{
    int count = 0;
    int slot;

    for (slot = 0; slot < BattleWorkPokeCountGet(bsys, battlerId); slot++) {
        struct PartyPokemon *mon = BattleWorkPokemonParamGet(bsys, battlerId, slot);
        if (mon != NULL && GetMonData(mon, MON_DATA_HP, NULL) != 0
            && GetMonData(mon, MON_DATA_STATUS, NULL) != 0) {
            count++;
        }
    }
    return count == 0 ? FAIR_AI_INVALID : count * 12 - incoming;
}

static u16 FairAI_StrongestPlayerAttack(struct BattleStruct *ctx, const FairAIKnownMon *player, const FairAIMonModel *trainer)
{
    u16 bestMove = MOVE_NONE;
    int bestDamage = -1;
    int i;

    for (i = 0; i < player->moveCount; i++) {
        u16 move = player->moves[i];
        int damage;

        if (ctx->moveTbl[move].split == SPLIT_STATUS) {
            continue;
        }
        damage = FairAI_Damage(ctx, player, trainer, move);
        if (damage > bestDamage) {
            bestDamage = damage;
            bestMove = move;
        }
    }
    return bestMove;
}

static int FairAI_StatusUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId,
    FairAIMonModel *trainer, const FairAIKnownMon *target, int targetBattler, u16 move, int incoming)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    struct BattlePokemon *liveTarget = targetBattler >= 0 ? &ctx->battlemon[targetBattler] : NULL;
    int ownSide = IsClientEnemy(bsys, battlerId);
    int targetSide = ownSide ^ 1;
    int accuracy = data->accuracy == 0 ? 100 : data->accuracy;
    int bestPhysical = FairAI_BestDamage(ctx, trainer, target, SPLIT_PHYSICAL);
    int bestSpecial = FairAI_BestDamage(ctx, trainer, target, SPLIT_SPECIAL);
    int bestDamage = bestPhysical > bestSpecial ? bestPhysical : bestSpecial;
    int stat;
    int change;
    BOOL targetsSelf;
    u8 targetType1 = target->type1;
    u8 targetType2 = target->type2;
    u8 effectClass = FairAI_GetEffectClass(data->effect);
    u8 moveClass = FairAI_GetMoveClass(move);

    if (FairAI_GetStageChange(data->effect, &stat, &change, &targetsSelf)) {
        int value = FairAI_StageUtility(ctx, trainer, target, targetBattler, stat, change, targetsSelf, incoming);
        if (!targetsSelf && value != FAIR_AI_INVALID && targetBattler >= 0 && CanSwitchMon(bsys, ctx, targetBattler)) {
            value = (value + incoming) * AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT / 100 - incoming;
        }
        return value == FAIR_AI_INVALID ? value : (value + incoming) * accuracy / 100 - incoming;
    }
    switch (effectClass) {
    case AI_EFFECT_CLASS_POISON:
    case AI_EFFECT_CLASS_TOXIC:
        if (target->condition || targetType1 == TYPE_POISON || targetType2 == TYPE_POISON
            || targetType1 == TYPE_STEEL || targetType2 == TYPE_STEEL) {
            return FAIR_AI_INVALID;
        }
        return ((effectClass == AI_EFFECT_CLASS_TOXIC ? 24 : 16) * accuracy / 100) - incoming;
    case AI_EFFECT_CLASS_BURN:
        if (target->condition) {
            return FAIR_AI_INVALID;
        }
        return (12 + FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL) / 2) * accuracy / 100 - incoming;
    case AI_EFFECT_CLASS_PARALYSIS:
        // Ground typing blocks Thunder Wave, but not Glare or powder-based
        // paralysis moves that share this effect code.
        if (target->condition || (data->type == TYPE_ELECTRIC && (targetType1 == TYPE_GROUND || targetType2 == TYPE_GROUND))) {
            return FAIR_AI_INVALID;
        }
        return (incoming / 4
                   + (FairAI_ApplyStage(trainer->speed, trainer->stages[STAT_SPEED])
                           <= FairAI_ApplyStage(target->speed, target->stages[STAT_SPEED])
                       ? bestDamage
                       : 0))
                * accuracy / 100
            - incoming;
    case AI_EFFECT_CLASS_SLEEP:
        if (target->condition
            || target->ability == ABILITY_INSOMNIA || target->ability == ABILITY_VITAL_SPIRIT
            || target->ability == ABILITY_SWEET_VEIL
            || (target->ability == ABILITY_LEAF_GUARD && (ctx->field_condition & FIELD_CONDITION_SUN_ALL))
            || (IsPowderMove(move)
                && (targetType1 == TYPE_GRASS || targetType2 == TYPE_GRASS
                    || target->ability == ABILITY_OVERCOAT
                    || BattleItemDataGet(ctx, target->item, 1) == HOLD_EFFECT_SPORE_POWDER_IMMUNITY))) {
            return FAIR_AI_INVALID;
        }
        {
            u16 playerMove = FairAI_StrongestPlayerAttack(ctx, target, trainer);
            int firstChance = playerMove == MOVE_NONE ? 100 : FairAI_ActsFirstChance(ctx, trainer, move, target, playerMove);
            int followUp = bestDamage * FAIR_AI_FUTURE_PERCENT / 100;
            int exposure = firstChance * (100 - accuracy) * incoming / 10000
                + (100 - firstChance) * incoming / 100;

            if (firstChance == 0 && incoming >= trainer->hp * 100 / (trainer->maxHp ? trainer->maxHp : 1)) {
                return FAIR_AI_INVALID;
            }
            return followUp * accuracy / 100 - exposure;
        }
    case AI_EFFECT_CLASS_DELAYEDSLEEP:
        if (target->condition || (liveTarget != NULL && (liveTarget->effect_of_moves & MOVE_EFFECT_FLAG_YAWN))) {
            return FAIR_AI_INVALID;
        }
        return incoming * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
    case AI_EFFECT_CLASS_LEECHSEED:
        if (targetType1 == TYPE_GRASS || targetType2 == TYPE_GRASS
            || (liveTarget != NULL && (liveTarget->effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED))) {
            return FAIR_AI_INVALID;
        }
        {
            int immediate = 12;
            int missingHp = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
            int persistence = targetBattler >= 0 && CanSwitchMon(bsys, ctx, targetBattler)
                ? AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT : 100;
            int future = 24 * persistence / 100;
            int tempo = bestDamage * (100 - persistence) / 100 * FAIR_AI_FUTURE_PERCENT / 100;

            if (missingHp > 0) {
                immediate += missingHp < 12 ? missingHp : 12;
            }
            return (immediate + future + tempo) * accuracy / 100 - incoming;
        }
    case AI_EFFECT_CLASS_RECOVERHALF: {
        int missing = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
        int healed = missing > 50 ? 50 : missing;
        return healed == 0 ? FAIR_AI_INVALID : healed - incoming;
    }
    case AI_EFFECT_CLASS_PROTECT: {
        int denominator;

        if (incoming == 0) {
            return FAIR_AI_INVALID;
        }
        denominator = FairAI_ProtectSuccessDenominator(ctx, battlerId);
        return -incoming * (denominator - 1) / denominator;
    }
    case AI_EFFECT_CLASS_ENDURE:
        return incoming < trainer->hpPercent ? FAIR_AI_INVALID : -incoming;
    case AI_EFFECT_CLASS_CURSE:
        return FairAI_CurseUtility(bsys, ctx, trainer, target, targetBattler, incoming);
    case AI_EFFECT_CLASS_CUREPARTYSTATUS:
        return FairAI_CurePartyStatusUtility(bsys, battlerId, incoming);
    case AI_EFFECT_CLASS_REST: {
        int missing = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
        int value = missing + (trainer->condition ? 12 : 0) - incoming;
        return missing == 0 && trainer->condition == 0 ? FAIR_AI_INVALID : value;
    }
    case AI_EFFECT_CLASS_TRICKROOM:
        return trainer->speed > FairAI_EstimateStat(target, PERSONAL_BASE_SPEED) ? FAIR_AI_INVALID : incoming / 2 - incoming;
    case AI_EFFECT_CLASS_ATTACKDEFENSEUP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_ATTACK, STAT_DEFENSE);
    case AI_EFFECT_CLASS_DEFENSESPECIALDEFENSEUP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_DEFENSE, STAT_SPECIAL_DEFENSE);
    case AI_EFFECT_CLASS_SPECIALATTACKSPECIALDEFENSEUP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_SPECIAL_ATTACK, STAT_SPECIAL_DEFENSE);
    case AI_EFFECT_CLASS_ATTACKSPEEDUP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_ATTACK, STAT_SPEED);
    case AI_EFFECT_CLASS_FLEE:
        return FAIR_AI_INVALID;
    default:
        break;
    }
    switch (moveClass) {
    case AI_MOVE_CLASS_TAILWIND:
        if (ctx->tailwindCount[ownSide] != 0) {
            return FAIR_AI_INVALID;
        }
        return trainer->speed <= target->speed && trainer->speed * 2 > target->speed
            ? bestDamage / 2 - incoming : FAIR_AI_INVALID;
    case AI_MOVE_CLASS_LIGHTSCREEN:
        return (ctx->side_condition[ownSide] & SIDE_STATUS_LIGHT_SCREEN) ? FAIR_AI_INVALID
            : FairAI_Threat(ctx, target, trainer, SPLIT_SPECIAL) / 2 - incoming;
    case AI_MOVE_CLASS_REFLECT:
        return (ctx->side_condition[ownSide] & SIDE_STATUS_REFLECT) ? FAIR_AI_INVALID
            : FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL) / 2 - incoming;
    case AI_MOVE_CLASS_SPIKES:
        return ctx->scw[targetSide].spikesLayers >= 3 ? FAIR_AI_INVALID : 12 - incoming;
    case AI_MOVE_CLASS_TOXICSPIKES:
        return ctx->scw[targetSide].toxicSpikesLayers >= 2 ? FAIR_AI_INVALID : 12 - incoming;
    case AI_MOVE_CLASS_STEALTHROCK:
        return (ctx->side_condition[targetSide] & SIDE_STATUS_STEALTH_ROCK) ? FAIR_AI_INVALID : 12 - incoming;
    case AI_MOVE_CLASS_STICKYWEB:
        return (ctx->side_condition[targetSide] & SIDE_STATUS_STICKY_WEB) ? FAIR_AI_INVALID : 10 - incoming;
    case AI_MOVE_CLASS_SUBSTITUTE:
        return trainer->hp * 100 / (trainer->maxHp ? trainer->maxHp : 1) <= 25 ? FAIR_AI_INVALID : -25;
    case AI_MOVE_CLASS_RANDOMACTION:
        return AI_CFG_RANDOM_ACTION_BASE_VALUE - incoming;
    default:
        return AI_CFG_UNKNOWN_ACTION_BASE_VALUE - incoming;
    }
}

static int FairAI_MoveUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId,
    FairAIMonModel *trainer, const FairAIKnownMon *target, int targetBattler, u16 move,
    BOOL playerSwitching)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    int incoming = playerSwitching ? 0 : FairAI_ExpectedThreat(ctx, target, trainer);

    if (data->split == SPLIT_STATUS) {
        return FairAI_StatusUtility(bsys, ctx, battlerId, trainer, target, targetBattler, move, incoming);
    } else {
        int damage = FairAI_Damage(ctx, trainer, target, move);
        int targetHp = target != NULL ? target->hpPercent : 100;
        int firstChance = 50;
        int playerMove = MOVE_NONE;

        if (target != NULL) {
            playerMove = FairAI_StrongestPlayerAttack(ctx, target, trainer);
        }
        if (playerMove != MOVE_NONE) {
            firstChance = FairAI_ActsFirstChance(ctx, trainer, move, target, playerMove);
        }
        if (damage > targetHp) {
            damage = targetHp;
        }
        if (damage >= targetHp) {
            incoming = incoming * (100 - firstChance) / 100;
        }
        if (damage < targetHp && firstChance != 0 && data->secondaryEffectChance != 0
            && FairAI_GetEffectClass(data->effect) == AI_EFFECT_CLASS_FLINCH) {
            damage += incoming * data->secondaryEffectChance / 100 * firstChance / 100;
        }
        return damage - incoming;
    }
}

static BOOL FairAI_IsLegalReserve(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, int slot)
{
    struct PartyPokemon *mon = BattleWorkPokemonParamGet(bsys, battlerId, slot);

    if (mon == NULL || GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG, NULL)
        || GetMonData(mon, MON_DATA_HP, NULL) == 0 || slot == ctx->sel_mons_no[battlerId]
        || slot == ctx->ai_reshuffle_sel_mons_no[battlerId]) {
        return FALSE;
    }
    return TRUE;
}

static BOOL FairAI_IsTrapped(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId)
{
    struct BattlePokemon *mon = &ctx->battlemon[battlerId];
    int opponent;

    if (mon->condition2 & (STATUS2_BIND | STATUS2_MEAN_LOOK) || mon->effect_of_moves & MOVE_EFFECT_FLAG_INGRAIN) {
        return TRUE;
    }
    // Hidden trapping abilities are consulted only as a command-legality oracle.
    for (opponent = 0; opponent < CLIENT_MAX; opponent++) {
        int ability;
        if (!IsBattlerSlotValid(bsys, opponent) || IsClientEnemy(bsys, opponent) == IsClientEnemy(bsys, battlerId)
            || ctx->battlemon[opponent].hp == 0) {
            continue;
        }
        ability = GetBattlerAbility(ctx, opponent);
        if (ability == ABILITY_SHADOW_TAG
            || (ability == ABILITY_MAGNET_PULL && (mon->type1 == TYPE_STEEL || mon->type2 == TYPE_STEEL))
            || (ability == ABILITY_ARENA_TRAP && mon->type1 != TYPE_FLYING && mon->type2 != TYPE_FLYING
                && mon->ability != ABILITY_LEVITATE && HeldItemHoldEffectGet(ctx, battlerId) != HOLD_EFFECT_UNGROUND_DESTROYED_ON_HIT)) {
            return TRUE;
        }
    }
    return FALSE;
}

static int FairAI_PlayerSwitchChance(const FairAIKnownMon *active, BOOL hasReserve, int trainerDamage)
{
    int chance = hasReserve ? 15 : 0;
    AIPredictionMemory *memory = &Sav2_Misc_get(SaveBlock2_get())->trainerAIMemory;

    if (active != NULL && active->hpPercent <= 50) {
        chance += 20;
    }
    if (active != NULL && trainerDamage >= active->hpPercent) {
        chance += 25;
    }
    if (memory->version == 1) {
        chance += memory->readBalance * 2;
    }
    if (!hasReserve) {
        return 0;
    }
    if (chance < 5) {
        chance = 5;
    }
    return chance > 70 ? 70 : chance;
}

static void FairAI_AssignProbabilityMass(FairAIPlayerAction *actions, const int *weights,
    int first, int count, int mass)
{
    int total = 0;
    int assigned = 0;
    int best = first;
    int i;

    if (count == 0 || mass == 0) {
        return;
    }
    for (i = first; i < first + count; i++) {
        total += weights[i];
        if (weights[i] > weights[best]) {
            best = i;
        }
    }
    if (total == 0) {
        total = count;
    }
    for (i = first; i < first + count; i++) {
        int weight = weights[i] == 0 ? 1 : weights[i];
        actions[i].probability = mass * weight / total;
        assigned += actions[i].probability;
    }
    actions[best].probability += mass - assigned;
}

static int FairAI_BuildPlayerActions(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int playerBattler, const FairAIMonModel *trainer, FairAIPlayerAction actions[AI_CFG_MAX_PLAYER_ACTIONS])
{
    const FairAIKnownMon *active = FairAI_GetRecord(ctx, playerBattler);
    int trainerBattler = FairAI_GetTarget(bsys, ctx, playerBattler);
    FairAIMoveState moveState;
    int weights[AI_CFG_MAX_PLAYER_ACTIONS];
    int moveDamage[MAX_MON_MOVES];
    int activeBalance;
    int bestDamage = 0;
    int moveCount = 0;
    int switchCount = 0;
    int actionCount = 0;
    int switchChance;
    int activeSlot = ctx->sel_mons_no[playerBattler];
    int slot;
    int i;

    if (active == NULL) {
        return 0;
    }
    FairAI_LoadMoveState(ctx, playerBattler, &moveState);
    if ((moveState.flags & FAIR_AI_MOVE_STATE_FORCED) && moveState.forcedMove != MOVE_NONE) {
        actions[0].kind = FAIR_AI_PLAYER_ACTION_MOVE;
        actions[0].value = moveState.forcedMove;
        actions[0].probability = AI_CFG_PROBABILITY_SCALE;
        return 1;
    }

    for (i = 0; i < active->moveCount && actionCount < AI_CFG_MAX_PLAYER_ACTIONS; i++) {
        u16 move = active->moves[i];
        int damage = FairAI_Damage(ctx, active, trainer, move);

        actions[actionCount].kind = FAIR_AI_PLAYER_ACTION_MOVE;
        actions[actionCount].value = move;
        actions[actionCount].probability = 0;
        moveDamage[moveCount++] = damage;
        if (damage > bestDamage) {
            bestDamage = damage;
        }
        actionCount++;
    }
    for (i = 0; i < moveCount; i++) {
        u16 move = actions[i].value;
        if (ctx->moveTbl[move].split == SPLIT_STATUS) {
            int utility = FairAI_MoveUtility(bsys, ctx, playerBattler,
                (FairAIMonModel *)active, trainer, trainerBattler, move, FALSE);
            weights[i] = utility == FAIR_AI_INVALID ? 1 : utility + 16;
            if (weights[i] < 1) {
                weights[i] = 1;
            } else if (weights[i] > 128) {
                weights[i] = 128;
            }
        } else {
            weights[i] = FairAI_DamageGapWeight(bestDamage - moveDamage[i]);
        }
    }

    activeBalance = FairAI_ExpectedThreat(ctx, active, trainer)
        - FairAI_BestDamage(ctx, trainer, active, -1);
    if (CanSwitchMon(bsys, ctx, playerBattler) && !FairAI_IsTrapped(bsys, ctx, playerBattler)) {
        int partyCount = BattleWorkPokeCountGet(bsys, playerBattler);
        for (slot = 0; slot < partyCount && slot < FAIR_AI_PARTY_SIZE
             && actionCount < AI_CFG_MAX_PLAYER_ACTIONS; slot++) {
            const FairAIKnownMon *candidate = &sFairAIState.playerMons[slot];
            int balance;
            int gain;

            if (slot == activeSlot || candidate->species == SPECIES_NONE || candidate->hp == 0) {
                continue;
            }
            balance = FairAI_ExpectedThreat(ctx, candidate, trainer)
                - FairAI_BestDamage(ctx, trainer, candidate, -1);
            gain = balance - activeBalance;
            actions[actionCount].kind = FAIR_AI_PLAYER_ACTION_SWITCH;
            actions[actionCount].value = slot;
            actions[actionCount].probability = 0;
            weights[actionCount] = gain > 0 ? 4 + gain : 1;
            actionCount++;
            switchCount++;
        }
    }

    switchChance = FairAI_PlayerSwitchChance(active, switchCount != 0, bestDamage);
    if (moveCount == 0) {
        switchChance = 100;
    } else if (switchCount == 0) {
        switchChance = 0;
    }
    FairAI_AssignProbabilityMass(actions, weights, 0, moveCount,
        AI_CFG_PROBABILITY_SCALE * (100 - switchChance) / 100);
    FairAI_AssignProbabilityMass(actions, weights, moveCount, switchCount,
        AI_CFG_PROBABILITY_SCALE - AI_CFG_PROBABILITY_SCALE * (100 - switchChance) / 100);
    return actionCount;
}

static int FairAI_AssetValue(const FairAIMonModel *mon)
{
    int hpPercent = mon->maxHp == 0 ? 0 : mon->hp * 100 / mon->maxHp;

    return hpPercent * AI_CFG_TEAM_ASSET_WEIGHT / 100 + mon->level / 2;
}

static int FairAI_PositionUtility(struct BattleStruct *ctx, FairAIMonModel *trainer,
    const FairAIKnownMon *target)
{
    int incoming = FairAI_ExpectedThreat(ctx, target, trainer);
    int hpPercent = trainer->maxHp == 0 ? 0 : trainer->hp * 100 / trainer->maxHp;
    return FairAI_BestDamage(ctx, trainer, target, -1) - incoming + hpPercent / 4;
}

static int FairAI_MoveUtilityAgainstMove(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int trainerBattler, FairAIMonModel *trainer, const FairAIKnownMon *player,
    int playerBattler, u16 trainerMove, u16 playerMove, const FairAIMoveState *playerState)
{
    const struct BattleMove *data = &ctx->moveTbl[trainerMove];
    int incoming = FairAI_DamageToTrainerAtState(ctx, player, trainer, playerMove, playerState);

    if (data->split == SPLIT_STATUS) {
        return FairAI_StatusUtility(bsys, ctx, trainerBattler, trainer,
            player, playerBattler, trainerMove, incoming);
    }
    {
        int damage = FairAI_Damage(ctx, trainer, player, trainerMove);
        int playerHp = player->hpPercent;
        int firstChance = FairAI_ActsFirstChance(ctx, trainer, trainerMove, player, playerMove);
        if (damage > playerHp) {
            damage = playerHp;
        }
        if (damage >= playerHp) {
            incoming = incoming * (100 - firstChance) / 100;
        }
        return damage - incoming;
    }
}

static int FairAI_ContinuationUtility(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int trainerBattler, FairAIMonModel *trainer,
    const FairAIKnownMon *player, int playerBattler, const FairAIMoveState *playerState)
{
    int best = FAIR_AI_INVALID;
    BOOL forced = playerState != NULL && (playerState->flags & FAIR_AI_MOVE_STATE_FORCED);
    int i;

    for (i = 0; i < MAX_MON_MOVES; i++) {
        u16 move = trainer->moves[i];
        int utility;
        if (move == MOVE_NONE || trainer->pp[i] == 0) {
            continue;
        }
        if (forced) {
            utility = FairAI_MoveUtilityAgainstMove(bsys, ctx, trainerBattler, trainer,
                player, playerBattler, move, playerState->forcedMove, playerState);
        } else {
            utility = FairAI_MoveUtility(bsys, ctx, trainerBattler, trainer,
                player, playerBattler, move, FALSE);
        }
        if (utility > best) {
            best = utility;
        }
    }
    if (best <= FAIR_AI_INVALID) {
        return 0;
    }
    return best * (forced ? AI_CFG_FORCED_FUTURE_PERCENT : AI_CFG_SPECULATIVE_FUTURE_PERCENT)
        / 100;
}

static int FairAI_EvaluateSwitchAgainstMove(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int trainerBattler, int trainerSlot, const FairAIKnownMon *player, int playerBattler,
    u16 playerMove, const FairAIMoveState *initialPlayerState)
{
    FairAIMonModel reserve;
    FairAIMonModel hitReserve;
    FairAIMoveState hitState = *initialPlayerState;
    int expectedDamage;
    int hitDamage;
    int hpPercent;
    int accuracy = ctx->moveTbl[playerMove].accuracy == 0 ? 100 : ctx->moveTbl[playerMove].accuracy;
    u8 moveType;
    u8 abilityBehavior;
    int hitUtility;
    int missUtility;

    FairAI_FillReserveModel(BattleWorkPokemonParamGet(bsys, trainerBattler, trainerSlot), &reserve);
    hitReserve = reserve;
    expectedDamage = FairAI_DamageToTrainerAtState(ctx, player, &reserve, playerMove, initialPlayerState);
    hitDamage = accuracy == 0 ? expectedDamage : expectedDamage * 100 / accuracy;
    hpPercent = reserve.maxHp == 0 ? 0 : reserve.hp * 100 / reserve.maxHp;
    moveType = GetAdjustedMoveTypeBasics(ctx, playerMove, player->ability,
        ctx->moveTbl[playerMove].type);
    abilityBehavior = FairAI_GetAbilityTypeBehavior(reserve.ability, moveType);
    if (abilityBehavior == AI_ABILITY_TYPE_ABSORBBOOST) {
        hitReserve.semanticState |= FAIR_AI_MON_STATE_FLASH_FIRE;
    }

    FairAI_AdvanceMoveStateOnHit(playerMove, initialPlayerState, &hitState);
    hitUtility = -hitDamage;
    if (hitDamage >= hpPercent) {
        hitUtility -= FairAI_AssetValue(&reserve);
    } else {
        hitUtility += FairAI_ContinuationUtility(bsys, ctx, trainerBattler, &hitReserve,
            player, playerBattler, &hitState);
    }
    missUtility = FairAI_ContinuationUtility(bsys, ctx, trainerBattler, &reserve,
        player, playerBattler, NULL);
    return (hitUtility * accuracy + missUtility * (100 - accuracy)) / 100;
}

static int FairAI_EvaluatePair(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int trainerBattler, FairAIMonModel *trainer, const FairAIKnownMon *player, int playerBattler,
    const FairAIAction *trainerAction, const FairAIPlayerAction *playerAction,
    const FairAIMoveState *playerState)
{
    if (trainerAction->kind == FAIR_AI_ACTION_MOVE
        && playerAction->kind == FAIR_AI_PLAYER_ACTION_MOVE) {
        u16 trainerMove = trainer->moves[trainerAction->index];
        int incoming = FairAI_DamageToTrainerAtState(ctx, player, trainer, playerAction->value, playerState);
        int utility = FairAI_MoveUtilityAgainstMove(bsys, ctx, trainerBattler, trainer,
            player, playerBattler, trainerMove, playerAction->value, playerState);
        int hpPercent = trainer->maxHp == 0 ? 0 : trainer->hp * 100 / trainer->maxHp;
        int outgoing = FairAI_Damage(ctx, trainer, player, trainerMove);
        int firstChance = FairAI_ActsFirstChance(ctx, trainer, trainerMove, player, playerAction->value);
        int accuracy = ctx->moveTbl[playerAction->value].accuracy == 0
            ? 100 : ctx->moveTbl[playerAction->value].accuracy;
        int hitDamage = incoming * 100 / accuracy;
        BOOL trainerFaints = hitDamage >= hpPercent
            && !(outgoing >= player->hpPercent && firstChance == 100);
        BOOL playerFaints = outgoing >= player->hpPercent && !trainerFaints;
        if (trainerFaints) {
            int faintChance = accuracy;
            u16 trainerMove = trainer->moves[trainerAction->index];
            u16 trainerEffect = ctx->moveTbl[trainerMove].effect;

            if (trainerEffect == MOVE_EFFECT_PROTECT || trainerEffect == MOVE_EFFECT_SURVIVE_WITH_1_HP) {
                int denominator = FairAI_ProtectSuccessDenominator(ctx, trainerBattler);
                faintChance = faintChance * (denominator - 1) / denominator;
            }
            utility -= FairAI_AssetValue(trainer) * faintChance / 100;
        } else if (!playerFaints) {
            if (FairAI_GetMoveTransition(playerAction->value) != 0) {
                FairAIMoveState nextPlayerState;
                int hitContinuation;
                int missContinuation;
                FairAI_AdvanceMoveStateOnHit(playerAction->value, playerState, &nextPlayerState);
                hitContinuation = FairAI_ContinuationUtility(bsys, ctx, trainerBattler, trainer,
                    player, playerBattler, &nextPlayerState);
                missContinuation = FairAI_ContinuationUtility(bsys, ctx, trainerBattler, trainer,
                    player, playerBattler, NULL);
                utility += (hitContinuation * accuracy + missContinuation * (100 - accuracy)) / 100;
            } else {
                utility += FairAI_ContinuationUtility(bsys, ctx, trainerBattler, trainer,
                    player, playerBattler, NULL);
            }
        }
        return utility;
    }
    if (trainerAction->kind == FAIR_AI_ACTION_MOVE) {
        const FairAIKnownMon *destination = &sFairAIState.playerMons[playerAction->value];
        int utility = FairAI_MoveUtility(bsys, ctx, trainerBattler, trainer,
            destination, -1, trainer->moves[trainerAction->index], TRUE);
        int outgoing = FairAI_Damage(ctx, trainer, destination,
            trainer->moves[trainerAction->index]);

        if (outgoing < destination->hpPercent) {
            utility += FairAI_ContinuationUtility(bsys, ctx, trainerBattler, trainer,
                destination, -1, NULL);
        }
        return utility;
    }
    if (playerAction->kind == FAIR_AI_PLAYER_ACTION_MOVE) {
        return FairAI_EvaluateSwitchAgainstMove(bsys, ctx, trainerBattler, trainerAction->index,
            player, playerBattler, playerAction->value, playerState);
    }
    {
        FairAIMonModel reserve;
        const FairAIKnownMon *destination = &sFairAIState.playerMons[playerAction->value];
        FairAI_FillReserveModel(BattleWorkPokemonParamGet(bsys, trainerBattler, trainerAction->index), &reserve);
        return FairAI_ContinuationUtility(bsys, ctx, trainerBattler, &reserve,
            destination, -1, NULL);
    }
}

static int FairAI_ScoreTrainerAction(struct BattleSystem *bsys, struct BattleStruct *ctx,
    int trainerBattler, FairAIMonModel *trainer, const FairAIKnownMon *player, int playerBattler,
    const FairAIAction *trainerAction, const FairAIPlayerAction *predictedActions, int playerActionCount,
    const FairAIMoveState *playerState, int riskWeight)
{
    s16 utilities[AI_CFG_MAX_PLAYER_ACTIONS];
    int expectedNumerator = 0;
    int downsideNumerator = 0;
    int expected;
    int i;

    for (i = 0; i < playerActionCount; i++) {
        utilities[i] = FairAI_EvaluatePair(bsys, ctx, trainerBattler, trainer, player, playerBattler,
            trainerAction, &predictedActions[i], playerState);
        expectedNumerator += predictedActions[i].probability * utilities[i];
    }
    expected = expectedNumerator / AI_CFG_PROBABILITY_SCALE;
    for (i = 0; i < playerActionCount; i++) {
        if (utilities[i] < expected) {
            downsideNumerator += predictedActions[i].probability * (expected - utilities[i]);
        }
    }
    expected -= downsideNumerator / AI_CFG_PROBABILITY_SCALE
        * riskWeight / AI_CFG_PROBABILITY_SCALE;
    return expected < FAIR_AI_INVALID ? FAIR_AI_INVALID : expected;
}

static void FairAI_Decide(struct BattleSystem *bsys, int battlerId)
{
    struct BattleStruct *ctx = bsys->sp;
    FairAIDecision *decision = &sFairAIState.decisions[battlerId];
    FairAIMonModel trainer;
    FairAIAction actions[FAIR_AI_MAX_ACTIONS];
    FairAIPlayerAction predictedActions[AI_CFG_MAX_PLAYER_ACTIONS];
    FairAIMoveState playerState;
    const FairAIKnownMon *target;
    u32 unusable;
    int targetBattler;
    int actionCount = 0;
    int playerActionCount;
    int bestFastMove = FAIR_AI_INVALID;
    int bestScore = FAIR_AI_INVALID;
    int incoming;
    int trainerHpPercent;
    int riskWeight;
    int i;
    u8 activeSlot = ctx->sel_mons_no[battlerId];

    FairAI_ResolveCommitment(bsys, battlerId);

    if (decision->valid && decision->turn == ctx->total_turn && decision->activeSlot == activeSlot) {
        return;
    }
    memset(decision, 0, sizeof(*decision));
    decision->turn = ctx->total_turn;
    decision->activeSlot = activeSlot;
    decision->command = 1;
    decision->move = 0xFF;
    decision->switchSlot = FAIR_AI_PARTY_SIZE;
    targetBattler = FairAI_GetTarget(bsys, ctx, battlerId);
    FairAI_RefreshKnowledge(bsys, ctx, targetBattler);
    FairAI_FillActiveModel(ctx, battlerId, &trainer);
    target = FairAI_GetRecord(ctx, targetBattler);
    unusable = TrainerAI_StruggleCheckReadOnly(bsys, ctx, battlerId);
    FairAI_LoadMoveState(ctx, targetBattler, &playerState);
    playerActionCount = FairAI_BuildPlayerActions(bsys, ctx, targetBattler, &trainer, predictedActions);

    for (i = 0; i < MAX_MON_MOVES; i++) {
        int fastScore;
        u16 move = trainer.moves[i];

        if (move == MOVE_NONE || trainer.pp[i] == 0 || (unusable & (1 << i))
            || (ctx->moveTbl[move].flag & FLAG_UNUSED_MOVE)) {
            continue;
        }
        fastScore = FairAI_MoveUtility(bsys, ctx, battlerId, &trainer,
            target, targetBattler, move, FALSE);
        actions[actionCount].kind = FAIR_AI_ACTION_MOVE;
        actions[actionCount].index = i;
        actions[actionCount].score = FAIR_AI_INVALID;
        actions[actionCount].fastScore = fastScore;
        {
            int pressure = FairAI_Damage(ctx, &trainer, target, move);
            actions[actionCount].pressureScore = pressure > 0x7FFF ? 0x7FFF : pressure;
        }
        if (fastScore > bestFastMove) {
            bestFastMove = fastScore;
        }
        actionCount++;
    }
    if (CanSwitchMon(bsys, ctx, battlerId) && !FairAI_IsTrapped(bsys, ctx, battlerId)) {
        int partyCount = BattleWorkPokeCountGet(bsys, battlerId);
        int currentIncoming = FairAI_ExpectedThreat(ctx, target, &trainer);
        int currentBalance = FairAI_PositionUtility(ctx, &trainer, target);
        int currentHp = trainer.maxHp == 0 ? 0 : trainer.hp * 100 / trainer.maxHp;
        s16 switchScores[AI_CFG_MAX_FULL_SWITCH_CANDIDATES] = { FAIR_AI_INVALID, FAIR_AI_INVALID };
        u8 switchSlots[AI_CFG_MAX_FULL_SWITCH_CANDIDATES] = { FAIR_AI_PARTY_SIZE, FAIR_AI_PARTY_SIZE };

        for (i = 0; i < partyCount && i < FAIR_AI_PARTY_SIZE; i++) {
            FairAIMonModel reserve;
            struct PartyPokemon *partyMon;
            int entryDamage;
            int hpPercent;
            int reserveBalance;
            int rank;

            if (!FairAI_IsLegalReserve(bsys, ctx, battlerId, i)) {
                continue;
            }
            partyMon = BattleWorkPokemonParamGet(bsys, battlerId, i);
            FairAI_FillReserveModel(partyMon, &reserve);
            entryDamage = FairAI_PredictedDamageIntoSwitch(ctx, target, &trainer, &reserve);
            hpPercent = reserve.maxHp == 0 ? 0 : reserve.hp * 100 / reserve.maxHp;
            if (entryDamage >= hpPercent) {
                continue;
            }
            reserveBalance = FairAI_PositionUtility(ctx, &reserve, target) - entryDamage;
            if (currentIncoming < currentHp
                && reserveBalance < currentBalance + AI_CFG_SWITCH_MIN_IMPROVEMENT) {
                continue;
            }
            for (rank = 0; rank < AI_CFG_MAX_FULL_SWITCH_CANDIDATES; rank++) {
                if (reserveBalance > switchScores[rank]) {
                    int moveRank;
                    for (moveRank = AI_CFG_MAX_FULL_SWITCH_CANDIDATES - 1;
                         moveRank > rank; moveRank--) {
                        switchScores[moveRank] = switchScores[moveRank - 1];
                        switchSlots[moveRank] = switchSlots[moveRank - 1];
                    }
                    switchScores[rank] = reserveBalance;
                    switchSlots[rank] = i;
                    break;
                }
            }
        }
        for (i = 0; i < AI_CFG_MAX_FULL_SWITCH_CANDIDATES; i++) {
            if (switchSlots[i] >= FAIR_AI_PARTY_SIZE) {
                break;
            }
            actions[actionCount].kind = FAIR_AI_ACTION_SWITCH;
            actions[actionCount].index = switchSlots[i];
            actions[actionCount].score = FAIR_AI_INVALID;
            actions[actionCount].fastScore = switchScores[i];
            actions[actionCount].pressureScore = 0;
            actionCount++;
        }
    }
    if (actionCount == 0 || playerActionCount == 0) {
        decision->valid = TRUE;
        return;
    }

    {
        incoming = FairAI_Threat(ctx, target, &trainer, -1);
        trainerHpPercent = trainer.maxHp == 0 ? 0 : trainer.hp * 100 / trainer.maxHp;
        if (incoming >= trainerHpPercent) {
            riskWeight = AI_CFG_DESPERATE_RISK_WEIGHT;
        } else if (target != NULL && trainerHpPercent >= target->hpPercent + 20) {
            riskWeight = AI_CFG_COMFORTABLE_RISK_WEIGHT;
        } else if (target != NULL && trainerHpPercent + 20 < target->hpPercent) {
            riskWeight = AI_CFG_BEHIND_RISK_WEIGHT;
        } else {
            riskWeight = AI_CFG_EVEN_RISK_WEIGHT;
        }
        for (i = 0; i < actionCount; i++) {
            actions[i].score = FairAI_ScoreTrainerAction(bsys, ctx, battlerId, &trainer,
                target, targetBattler, &actions[i], predictedActions, playerActionCount,
                &playerState, riskWeight);
        }
        {
            int bestAction = 0;
            int bestPressure = -1;

            for (i = 0; i < actionCount; i++) {
                if (actions[i].score > bestScore
                    || (actions[i].score == bestScore
                        && actions[i].pressureScore > bestPressure)) {
                    bestScore = actions[i].score;
                    bestPressure = actions[i].pressureScore;
                    bestAction = i;
                }
            }
            i = bestAction;
        }
    }
    decision->valid = TRUE;
    decision->target = targetBattler;
    if (actions[i].kind == FAIR_AI_ACTION_SWITCH) {
        decision->command = 3;
        decision->switchSlot = actions[i].index & 0xFF;
        ctx->ai_reshuffle_sel_mons_no[battlerId] = decision->switchSlot;
    } else {
        decision->move = actions[i].index & 0xFF;
        ctx->aiWorkTable.ai_dir_select_client[battlerId] = targetBattler;
        if (actions[i].fastScore < bestFastMove - AI_CFG_READ_COMMITMENT_GAP
            && actions[i].score >= actions[i].fastScore + AI_CFG_HARD_READ_MIN_PAYOFF) {
            FairAICommitment *commitment = &sFairAIState.commitments[battlerId];
            commitment->valid = TRUE;
            commitment->turn = ctx->total_turn;
            commitment->playerBattler = targetBattler;
            commitment->playerSlot = ctx->sel_mons_no[targetBattler];
            commitment->situation = FairAI_SituationClass(&trainer, target);
            commitment->actionClass = 1;
        }
    }
}

u8 FairTrainerAI_PickCommand(struct BattleSystem *bsys, u8 battlerId)
{
    if (!FairAI_IsSupportedBattle(bsys, battlerId) || sFairAIStatePtr == NULL) {
        return TrainerAI_PickCommand_Original(bsys, battlerId);
    }
    FairAI_Decide(bsys, battlerId);
    return sFairAIState.decisions[battlerId].command;
}

u8 FairTrainerAI_PickMove(struct BattleSystem *bsys, u8 battlerId)
{
    if (!FairAI_IsSupportedBattle(bsys, battlerId) || sFairAIStatePtr == NULL) {
        return TrainerAI_PickMove_Original(bsys, battlerId);
    }
    FairAI_Decide(bsys, battlerId);
    return sFairAIState.decisions[battlerId].move;
}

u8 FairTrainerAI_PickReplacement(struct BattleSystem *bsys, u8 battlerId)
{
    struct BattleStruct *ctx = bsys->sp;
    const FairAIKnownMon *target;
    int targetBattler;
    int bestSlot = FAIR_AI_PARTY_SIZE;
    int bestUtility = FAIR_AI_INVALID;
    int partyCount;
    int slot;

    if (!FairAI_IsSupportedBattle(bsys, battlerId) || sFairAIStatePtr == NULL || ctx->total_turn == 0
        || !sFairAIState.decisions[battlerId].valid
        || ctx->sel_mons_no[battlerId] >= FAIR_AI_PARTY_SIZE
        || ctx->battlemon[battlerId].species == SPECIES_NONE
        || ctx->battlemon[battlerId].hp != 0) {
        return TrainerAI_PickReplacement_Original(bsys, battlerId);
    }

    targetBattler = FairAI_GetTarget(bsys, ctx, battlerId);
    FairAI_RefreshKnowledge(bsys, ctx, targetBattler);
    target = FairAI_GetRecord(ctx, targetBattler);
    partyCount = BattleWorkPokeCountGet(bsys, battlerId);
    for (slot = 0; slot < partyCount && slot < FAIR_AI_PARTY_SIZE; slot++) {
        FairAIMonModel reserve;
        int utility;

        if (!FairAI_IsLegalReserve(bsys, ctx, battlerId, slot)) {
            continue;
        }
        FairAI_FillReserveModel(BattleWorkPokemonParamGet(bsys, battlerId, slot), &reserve);
        utility = FairAI_PositionUtility(ctx, &reserve, target);
        if (utility > bestUtility) {
            bestUtility = utility;
            bestSlot = slot;
        }
    }
    return bestSlot == FAIR_AI_PARTY_SIZE ? TrainerAI_PickReplacement_Original(bsys, battlerId) : bestSlot;
}

#endif // IMPLEMENT_EXPERT_TRAINER_AI
