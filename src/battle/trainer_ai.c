#include "battle.h"
#include "config.h"
#include "pokemon.h"
#include "trainer_ai.h"
#include "trainer_data.h"
#include "types.h"
#include "constants/ability.h"
#include "constants/battle_constants.h"
#include "constants/generated/learnsets.h"
#include "constants/hold_item_effects.h"
#include "constants/move_effects.h"
#include "constants/moves.h"
#include "constants/species.h"

#ifdef IMPLEMENT_FAIR_TRAINER_AI

#define FAIR_AI_PARTY_SIZE           6
#define FAIR_AI_PREDICTED_MOVES      8
#define FAIR_AI_MAX_ACTIONS          9
#define FAIR_AI_OBSERVED_WEIGHT      255
#define FAIR_AI_FUTURE_PERCENT       60
#define FAIR_AI_NEAR_BEST_MARGIN     8
#define FAIR_AI_RECENT_SWITCH_TURNS  2
#define FAIR_AI_INVALID              (-30000)
#define FAIR_AI_UNKNOWN              (-29999)

typedef struct FairAIKnownMon {
    u16 species;
    u16 moves[FAIR_AI_PREDICTED_MOVES];
    u16 ability;
    u16 item;
    u8 moveWeights[FAIR_AI_PREDICTED_MOVES];
    u8 moveCount;
    u8 form;
    u8 level;
    u8 hpPercent;
    u32 condition;
} FairAIKnownMon;

typedef struct FairAIMonModel {
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u16 ability;
    u16 hp;
    u16 maxHp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 spAttack;
    u16 spDefense;
    u8 pp[MAX_MON_MOVES];
    u8 stages[8];
    u8 type1;
    u8 type2;
    u8 form;
    u8 level;
    u32 condition;
    u32 condition2;
    u32 effectFlags;
} FairAIMonModel;

typedef enum FairAIActionKind {
    FAIR_AI_ACTION_MOVE,
    FAIR_AI_ACTION_SWITCH,
} FairAIActionKind;

typedef struct FairAIAction {
    s16 stayUtility;
    s16 switchUtility;
    u16 weight;
    u8 kind;
    u8 index;
} FairAIAction;

typedef struct FairAIDecision {
    u16 turn;
    u8 valid;
    u8 activeSlot;
    u8 command;
    u8 move;
    u8 target;
    u8 switchSlot;
} FairAIDecision;

typedef struct FairAIState {
    FairAIKnownMon playerMons[FAIR_AI_PARTY_SIZE];
    FairAIDecision decisions[CLIENT_MAX];
    u8 lastSwitchedOut[CLIENT_MAX];
    u8 lastSwitchTurn[CLIENT_MAX];
    u8 observedActiveSlot[CLIENT_MAX];
    u8 actedSinceEntry[CLIENT_MAX];
} FairAIState;

static FairAIState sFairAIState;

static u8 FairAI_HpPercent(s32 hp, u32 maxHp)
{
    if (hp <= 0 || maxHp == 0) {
        return 0;
    }
    // Quantize the visible HP bar rather than retaining the player's exact HP.
    return ((u32)hp * 8 / maxHp) * 12 + 4;
}

static FairAIKnownMon *FairAI_GetRecord(struct BattleStruct *ctx, int battlerId)
{
    u8 slot = ctx->sel_mons_no[battlerId];

    return slot < FAIR_AI_PARTY_SIZE ? &sFairAIState.playerMons[slot] : NULL;
}

static void FairAI_AddPredictedMove(FairAIKnownMon *known, u16 move, u8 weight, BOOL observed)
{
    int i;

    if (move == MOVE_NONE || move > NUM_OF_MOVES) {
        return;
    }
    for (i = 0; i < known->moveCount; i++) {
        if (known->moves[i] == move) {
            if (observed) {
                known->moveWeights[i] = FAIR_AI_OBSERVED_WEIGHT;
            }
            return;
        }
    }
    if (known->moveCount == FAIR_AI_PREDICTED_MOVES) {
        volatile u16 *moves = known->moves;
        volatile u8 *weights = known->moveWeights;
        for (i = 1; i < FAIR_AI_PREDICTED_MOVES; i++) {
            moves[i - 1] = moves[i];
            weights[i - 1] = weights[i];
        }
        known->moveCount--;
    }
    i = known->moveCount++;
    known->moves[i] = move;
    known->moveWeights[i] = observed ? FAIR_AI_OBSERVED_WEIGHT : weight;
}

static void FairAI_LoadNaturalMoves(FairAIKnownMon *known)
{
    u32 learnset[MAX_LEVELUP_MOVES];
    int i;

    known->moveCount = 0;
    LoadLevelUpLearnset_HandleAlternateForm(known->species, known->form, learnset);
    for (i = 0; i < MAX_LEVELUP_MOVES; i++) {
        u32 entry = learnset[i];
        u16 move = LEVEL_UP_LEARNSET_MOVE(entry);
        u16 level = LEVEL_UP_LEARNSET_LEVEL(entry);

        if (move == LEVEL_UP_LEARNSET_END) {
            break;
        }
        if (level == 0 || level <= known->level) {
            FairAI_AddPredictedMove(known, move, 48, FALSE);
        }
    }
    for (i = 0; i < known->moveCount; i++) {
        known->moveWeights[i] = 48 + i * 8;
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
    if (known->species != mon->species || known->form != mon->form_no) {
        memset(known, 0, sizeof(*known));
        known->species = mon->species;
        known->form = mon->form_no;
        known->level = mon->level;
        FairAI_LoadNaturalMoves(known);
    }
    known->level = mon->level;
    known->hpPercent = FairAI_HpPercent(mon->hp, mon->maxhp);
    known->condition = mon->condition;
    FairAI_AddPredictedMove(known, ctx->waza_no_old[battlerId], FAIR_AI_OBSERVED_WEIGHT, TRUE);
    for (i = 0; i < MAX_MON_MOVES; i++) {
        FairAI_AddPredictedMove(known, ctx->aiWorkTable.ai_defence_use_move[battlerId][i], FAIR_AI_OBSERVED_WEIGHT, TRUE);
    }
}

static void FairAI_RefreshKnowledge(struct BattleSystem *bsys, struct BattleStruct *ctx)
{
    int battlerId;

    for (battlerId = 0; battlerId < CLIENT_MAX; battlerId++) {
        FairAI_RecordActive(bsys, ctx, battlerId);
    }
}

BOOL TrainerAI_UsesStrategicLayer(u32 aiFlags)
{
    return (aiFlags & F_TRAINER_EXPERT_AI) == F_TRAINER_EXPERT_AI;
}

void FairTrainerAI_Reset(struct BattleSystem *bsys)
{
    int i;

    memset(&sFairAIState, 0, sizeof(sFairAIState));
    for (i = 0; i < CLIENT_MAX; i++) {
        sFairAIState.lastSwitchedOut[i] = FAIR_AI_PARTY_SIZE;
        sFairAIState.observedActiveSlot[i] = bsys->sp->sel_mons_no[i];
        // Initial leads were deliberately selected before turn one. Only a
        // later entry needs to commit to an action before switching again.
        sFairAIState.actedSinceEntry[i] = TRUE;
    }
}

void FairTrainerAI_ObserveAbility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, u16 ability)
{
    FairAIKnownMon *known;

    if (!ClientBelongsToPlayer(bsys, battlerId)) {
        return;
    }
    FairAI_RecordActive(bsys, ctx, battlerId);
    known = FairAI_GetRecord(ctx, battlerId);
    if (known != NULL) {
        known->ability = ability;
    }
}

void FairTrainerAI_ObserveHeldItem(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, u16 item)
{
    FairAIKnownMon *known;

    if (!ClientBelongsToPlayer(bsys, battlerId)) {
        return;
    }
    FairAI_RecordActive(bsys, ctx, battlerId);
    known = FairAI_GetRecord(ctx, battlerId);
    if (known != NULL) {
        known->item = item;
    }
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
    int base = PokeFormNoPersonalParaGet(mon->species, mon->form, field);
    int value = ((2 * base + 15) * mon->level) / 100;

    return value + (field == PERSONAL_BASE_HP ? mon->level + 10 : 5);
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
    if ((moveType == TYPE_GROUND && ability == ABILITY_LEVITATE)
        || (moveType == TYPE_FIRE && ability == ABILITY_FLASH_FIRE)
        || (moveType == TYPE_WATER && (ability == ABILITY_WATER_ABSORB || ability == ABILITY_STORM_DRAIN || ability == ABILITY_DRY_SKIN))
        || (moveType == TYPE_ELECTRIC && (ability == ABILITY_VOLT_ABSORB || ability == ABILITY_LIGHTNING_ROD || ability == ABILITY_MOTOR_DRIVE))
        || (moveType == TYPE_GRASS && ability == ABILITY_SAP_SIPPER)) {
        return 0;
    }
    if ((moveType == TYPE_FIRE || moveType == TYPE_ICE) && ability == ABILITY_THICK_FAT) {
        return 50;
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
        return target->hpPercent / 2;
    case MOVE_FINAL_GAMBIT:
        return attacker->hp * 100 / targetHp;
    case MOVE_GUILLOTINE:
    case MOVE_HORN_DRILL:
    case MOVE_FISSURE:
    case MOVE_SHEER_COLD:
        return target->hpPercent * 30 / 100;
    default:
        return FAIR_AI_UNKNOWN;
    }
}

static int FairAI_DamageToKnown(struct BattleStruct *ctx, const FairAIMonModel *attacker, const FairAIKnownMon *target, u16 move)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    int attack;
    int defense;
    int targetHp;
    int damage;
    int typeFactor;
    int accuracy;
    u8 moveType;
    u16 ability1;
    u16 ability2;
    int factor1;
    int factor2;

    if (data->split == SPLIT_STATUS || target == NULL || target->species == SPECIES_NONE) {
        return FAIR_AI_INVALID;
    }
    if (data->power == 0) {
        return FairAI_FixedDamage(attacker, target, move);
    }
    moveType = GetAdjustedMoveTypeBasics(ctx, move, attacker->ability, data->type);
    ability1 = target->ability;
    ability2 = target->ability;
    if (ability1 == ABILITY_NONE) {
        ability1 = PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_ABILITY_1);
        ability2 = PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_ABILITY_2);
    }
    factor1 = FairAI_TypeFactor(moveType,
        PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_1),
        PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_2), ability1);
    factor2 = FairAI_TypeFactor(moveType,
        PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_1),
        PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_2), ability2);
    typeFactor = factor1 < factor2 ? factor1 : factor2;
    if (typeFactor == 0) {
        return 0;
    }
    if (data->split == SPLIT_PHYSICAL) {
        attack = FairAI_ApplyStage(attacker->attack, attacker->stages[STAT_ATTACK]);
        defense = FairAI_EstimateStat(target, PERSONAL_BASE_DEFENSE);
    } else {
        attack = FairAI_ApplyStage(attacker->spAttack, attacker->stages[STAT_SPECIAL_ATTACK]);
        defense = FairAI_EstimateStat(target, PERSONAL_BASE_SP_DEFENSE);
    }
    targetHp = FairAI_EstimateStat(target, PERSONAL_BASE_HP);
    damage = (((2 * attacker->level / 5 + 2) * data->power * attack / (defense ? defense : 1)) / 50) + 2;
    damage = damage * typeFactor / 100;
    if (moveType == attacker->type1 || moveType == attacker->type2) {
        damage = damage * 3 / 2;
    }
    accuracy = data->accuracy == 0 ? 100 : data->accuracy;
    return damage * 100 / targetHp * accuracy / 100;
}

static int FairAI_DamageToTrainer(struct BattleStruct *ctx, const FairAIKnownMon *attacker, const FairAIMonModel *target, u16 move)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    int attack;
    int defense;
    int damage;
    int typeFactor;
    int accuracy;
    u8 moveType;

    if (data->split == SPLIT_STATUS || attacker == NULL || attacker->species == SPECIES_NONE) {
        return 0;
    }
    if (data->power == 0) {
        switch (move) {
        case MOVE_SONIC_BOOM:
            return 2000 / (target->maxHp ? target->maxHp : 1);
        case MOVE_DRAGON_RAGE:
            return 4000 / (target->maxHp ? target->maxHp : 1);
        case MOVE_SEISMIC_TOSS:
        case MOVE_NIGHT_SHADE:
        case MOVE_PSYWAVE:
            return attacker->level * 100 / (target->maxHp ? target->maxHp : 1);
        case MOVE_SUPER_FANG:
            return target->hp * 50 / (target->maxHp ? target->maxHp : 1);
        default:
            return 0;
        }
    }
    moveType = data->type;
    typeFactor = FairAI_TypeFactor(moveType, target->type1, target->type2, target->ability);
    if (typeFactor == 0) {
        return 0;
    }
    if (data->split == SPLIT_PHYSICAL) {
        attack = FairAI_EstimateStat(attacker, PERSONAL_BASE_ATTACK);
        defense = FairAI_ApplyStage(target->defense, target->stages[STAT_DEFENSE]);
    } else {
        attack = FairAI_EstimateStat(attacker, PERSONAL_BASE_SP_ATTACK);
        defense = FairAI_ApplyStage(target->spDefense, target->stages[STAT_SPECIAL_DEFENSE]);
    }
    damage = (((2 * attacker->level / 5 + 2) * data->power * attack / (defense ? defense : 1)) / 50) + 2;
    damage = damage * typeFactor / 100;
    if (moveType == PokeFormNoPersonalParaGet(attacker->species, attacker->form, PERSONAL_TYPE_1)
        || moveType == PokeFormNoPersonalParaGet(attacker->species, attacker->form, PERSONAL_TYPE_2)) {
        damage = damage * 3 / 2;
    }
    accuracy = data->accuracy == 0 ? 100 : data->accuracy;
    return damage * 100 / (target->maxHp ? target->maxHp : 1) * accuracy / 100;
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
        damage = FairAI_DamageToTrainer(ctx, attacker, target, move);
        damage = damage * attacker->moveWeights[i] / FAIR_AI_OBSERVED_WEIGHT;
        if (damage > best) {
            best = damage;
        }
    }
    return best;
}

static int FairAI_EffectivePriority(struct BattleStruct *ctx, const FairAIMonModel *mon, u16 move)
{
    int priority = ctx->moveTbl[move].priority;
    u16 effect = ctx->moveTbl[move].effect;

    if (mon->ability == ABILITY_PRANKSTER && ctx->moveTbl[move].split == SPLIT_STATUS) {
        priority++;
    }
    if (mon->ability == ABILITY_GALE_WINGS && ctx->moveTbl[move].type == TYPE_FLYING && mon->hp == mon->maxHp) {
        priority++;
    }
    if (mon->ability == ABILITY_TRIAGE
        && (effect == MOVE_EFFECT_RESTORE_HALF_HP || effect == MOVE_EFFECT_HEAL_HALF_DIFFERENT_IN_WEATHER || effect == MOVE_EFFECT_STATUS_LEECH_SEED)) {
        priority += 3;
    }
    return priority;
}

static int FairAI_ActsFirstChance(struct BattleStruct *ctx, const FairAIMonModel *trainer, u16 trainerMove, const FairAIKnownMon *player, u16 playerMove)
{
    int trainerPriority = FairAI_EffectivePriority(ctx, trainer, trainerMove);
    int playerPriority = ctx->moveTbl[playerMove].priority;
    int trainerSpeed;
    int playerSpeed;

    if (player->ability == ABILITY_PRANKSTER && ctx->moveTbl[playerMove].split == SPLIT_STATUS) {
        playerPriority++;
    }
    if (trainerPriority != playerPriority) {
        return trainerPriority > playerPriority ? 100 : 0;
    }
    trainerSpeed = FairAI_ApplyStage(trainer->speed, trainer->stages[STAT_SPEED]);
    playerSpeed = FairAI_EstimateStat(player, PERSONAL_BASE_SPEED);
    if (ctx->field_condition & FIELD_CONDITION_TRICK_ROOM) {
        return trainerSpeed < playerSpeed ? 75 : trainerSpeed > playerSpeed ? 25 : 50;
    }
    return trainerSpeed > playerSpeed ? 75 : trainerSpeed < playerSpeed ? 25 : 50;
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
        damage = FairAI_DamageToKnown(ctx, attacker, target, move);
        if (damage > best && damage > FAIR_AI_UNKNOWN) {
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
            before = FairAI_BestDamage(ctx, trainer, target, stat == STAT_ATTACK ? SPLIT_PHYSICAL : SPLIT_SPECIAL);
            after = before * StatBoostModifiers[stage + change > 12 ? 12 : stage + change][0]
                * StatBoostModifiers[stage][1]
                / (StatBoostModifiers[stage + change > 12 ? 12 : stage + change][1] * StatBoostModifiers[stage][0]);
            return (after - before) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
        }
        if (stat == STAT_DEFENSE || stat == STAT_SPECIAL_DEFENSE) {
            before = incoming;
            after = before * StatBoostModifiers[stage][0]
                * StatBoostModifiers[stage + change > 12 ? 12 : stage + change][1]
                / (StatBoostModifiers[stage][1] * StatBoostModifiers[stage + change > 12 ? 12 : stage + change][0]);
            return (before - after) * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
        }
        if (stat == STAT_SPEED) {
            int targetSpeed = FairAI_EstimateStat(target, PERSONAL_BASE_SPEED);
            int oldSpeed = FairAI_ApplyStage(trainer->speed, stage);
            int newSpeed = FairAI_ApplyStage(trainer->speed, stage + change);
            return ((oldSpeed <= targetSpeed && newSpeed > targetSpeed) ? incoming : 0) - incoming;
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
        before = trainer->speed <= FairAI_EstimateStat(target, PERSONAL_BASE_SPEED) ? incoming : 0;
        return before * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
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

static int FairAI_CombinedSelfUtility(struct BattleStruct *ctx, FairAIMonModel *trainer, const FairAIKnownMon *target, int incoming, int stat1, int change1, int stat2, int change2)
{
    int first = FairAI_StageUtility(ctx, trainer, target, -1, stat1, change1, TRUE, incoming);
    int second = FairAI_StageUtility(ctx, trainer, target, -1, stat2, change2, TRUE, incoming);

    if (first <= FAIR_AI_UNKNOWN || second <= FAIR_AI_UNKNOWN) {
        return FAIR_AI_INVALID;
    }
    // StageUtility charges the cost of taking the incoming hit. Charge it once
    // when a single move raises two stats.
    return first + second + incoming;
}

static int FairAI_TargetPositiveStages(struct BattleStruct *ctx, int targetBattler)
{
    int total = 0;
    int i;

    if (targetBattler < 0) {
        return 0;
    }
    for (i = STAT_ATTACK; i <= STAT_EVASION; i++) {
        if (ctx->battlemon[targetBattler].states[i] > 6) {
            total += ctx->battlemon[targetBattler].states[i] - 6;
        }
    }
    return total;
}

static int FairAI_OwnPositiveStages(const FairAIMonModel *trainer)
{
    int total = 0;
    int i;

    for (i = STAT_ATTACK; i <= STAT_EVASION; i++) {
        if (trainer->stages[i] > 6) {
            total += trainer->stages[i] - 6;
        }
    }
    return total;
}

static int FairAI_TeamSupportValue(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, const FairAIMonModel *source, int sourceSlot, const FairAIKnownMon *target, int split)
{
    int targetSpeed = FairAI_EstimateStat(target, PERSONAL_BASE_SPEED);
    int partyCount = BattleWorkPokeCountGet(bsys, battlerId);
    int value = 0;
    int slot;

    for (slot = 0; slot < partyCount && slot < FAIR_AI_PARTY_SIZE; slot++) {
        FairAIMonModel teammate;
        struct PartyPokemon *partyMon;
        int contribution;

        if (slot == sourceSlot) {
            teammate = *source;
        } else {
            partyMon = BattleWorkPokemonParamGet(bsys, battlerId, slot);
            if (partyMon == NULL || GetMonData(partyMon, MON_DATA_SPECIES, NULL) == SPECIES_NONE
                || GetMonData(partyMon, MON_DATA_IS_EGG, NULL) || GetMonData(partyMon, MON_DATA_HP, NULL) == 0) {
                continue;
            }
            FairAI_FillReserveModel(partyMon, &teammate);
        }
        if (split < 0) {
            int speed = FairAI_ApplyStage(teammate.speed, teammate.stages[STAT_SPEED]);
            if (speed > targetSpeed || speed * 2 <= targetSpeed) {
                continue;
            }
            // Moving first converts roughly half of the teammate's best
            // expected hit from exposed damage into pressure.
            contribution = FairAI_BestDamage(ctx, &teammate, target, -1) / 2;
        } else {
            contribution = FairAI_Threat(ctx, target, &teammate, split) / 2;
        }
        // Reserve benefits are discounted because that teammate may not enter
        // before the temporary team effect expires.
        value += slot == sourceSlot ? contribution : contribution * FAIR_AI_FUTURE_PERCENT / 100;
    }
    return value;
}

static int FairAI_StatusUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, FairAIMonModel *trainer, int trainerSlot, const FairAIKnownMon *target, int targetBattler, u16 move, int incoming)
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
    u8 targetType1 = PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_1);
    u8 targetType2 = PokeFormNoPersonalParaGet(target->species, target->form, PERSONAL_TYPE_2);

    if (FairAI_GetStageChange(data->effect, &stat, &change, &targetsSelf)) {
        int value = FairAI_StageUtility(ctx, trainer, target, targetBattler, stat, change, targetsSelf, incoming);
        return value <= FAIR_AI_UNKNOWN ? value : value * accuracy / 100;
    }
    switch (data->effect) {
    case MOVE_EFFECT_STATUS_POISON:
    case MOVE_EFFECT_STATUS_BADLY_POISON:
        if (target->condition || targetType1 == TYPE_POISON || targetType2 == TYPE_POISON
            || targetType1 == TYPE_STEEL || targetType2 == TYPE_STEEL) {
            return FAIR_AI_INVALID;
        }
        return ((data->effect == MOVE_EFFECT_STATUS_BADLY_POISON ? 24 : 16) * accuracy / 100) - incoming;
    case MOVE_EFFECT_STATUS_BURN:
        if (target->condition) {
            return FAIR_AI_INVALID;
        }
        return (12 + FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL) / 2) * accuracy / 100 - incoming;
    case MOVE_EFFECT_STATUS_PARALYZE:
        // Ground typing blocks Thunder Wave, but not Glare or powder-based
        // paralysis moves that share this effect code.
        if (target->condition || (data->type == TYPE_ELECTRIC && (targetType1 == TYPE_GROUND || targetType2 == TYPE_GROUND))) {
            return FAIR_AI_INVALID;
        }
        return (incoming / 4 + (trainer->speed <= FairAI_EstimateStat(target, PERSONAL_BASE_SPEED) ? bestDamage : 0)) * accuracy / 100 - incoming;
    case MOVE_EFFECT_STATUS_SLEEP:
        if (target->condition) {
            return FAIR_AI_INVALID;
        }
        return incoming * 3 / 2 * accuracy / 100 - incoming;
    case MOVE_EFFECT_STATUS_SLEEP_NEXT_TURN:
        if (target->condition || (liveTarget != NULL && (liveTarget->effect_of_moves & MOVE_EFFECT_FLAG_YAWN))) {
            return FAIR_AI_INVALID;
        }
        return incoming * FAIR_AI_FUTURE_PERCENT / 100 - incoming;
    case MOVE_EFFECT_STATUS_LEECH_SEED:
        if (targetType1 == TYPE_GRASS || targetType2 == TYPE_GRASS
            || (liveTarget != NULL && (liveTarget->effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED))) {
            return FAIR_AI_INVALID;
        }
        return 24 * accuracy / 100 - incoming;
    case MOVE_EFFECT_RESTORE_HALF_HP:
    case MOVE_EFFECT_HEAL_HALF_DIFFERENT_IN_WEATHER: {
        int missing = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
        int healed = missing > 50 ? 50 : missing;
        return healed == 0 ? FAIR_AI_INVALID : healed - incoming;
    }
    case MOVE_EFFECT_PROTECT:
        return incoming == 0 ? FAIR_AI_INVALID : incoming;
    case MOVE_EFFECT_RECOVER_HEALTH_AND_SLEEP: {
        int missing = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
        int value = missing + (trainer->condition ? 12 : 0) - incoming;
        return missing == 0 && trainer->condition == 0 ? FAIR_AI_INVALID : value;
    }
    case MOVE_EFFECT_HEAL_HALF_REMOVE_FLYING_TYPE: {
        int missing = trainer->maxHp == 0 ? 0 : 100 - trainer->hp * 100 / trainer->maxHp;
        int healed = missing > 50 ? 50 : missing;
        return healed == 0 ? FAIR_AI_INVALID : healed - incoming;
    }
    case MOVE_EFFECT_ENCORE:
        return targetBattler >= 0 && ctx->waza_no_old[targetBattler] != MOVE_NONE ? incoming / 2 - incoming : FAIR_AI_INVALID;
    case MOVE_EFFECT_WEATHER_RAIN:
    case MOVE_EFFECT_WEATHER_SUN:
    case MOVE_EFFECT_WEATHER_SANDSTORM:
    case MOVE_EFFECT_WEATHER_HAIL:
    case MOVE_EFFECT_WEATHER_SNOW:
        return bestDamage / 3 - incoming;
    case MOVE_EFFECT_TRICK_ROOM:
        return trainer->speed > FairAI_EstimateStat(target, PERSONAL_BASE_SPEED) ? FAIR_AI_INVALID : incoming / 2 - incoming;
    case MOVE_EFFECT_ATK_DEF_UP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_ATTACK, 1, STAT_DEFENSE, 1);
    case MOVE_EFFECT_DEF_SP_DEF_UP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_DEFENSE, 1, STAT_SPECIAL_DEFENSE, 1);
    case MOVE_EFFECT_SP_ATK_SP_DEF_UP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_SPECIAL_ATTACK, 1, STAT_SPECIAL_DEFENSE, 1);
    case MOVE_EFFECT_ATK_SPEED_UP:
        return FairAI_CombinedSelfUtility(ctx, trainer, target, incoming, STAT_ATTACK, 1, STAT_SPEED, 1);
    case MOVE_EFFECT_DEF_UP_DOUBLE_ROLLOUT_POWER:
        return FairAI_StageUtility(ctx, trainer, target, targetBattler, STAT_DEFENSE, 1, TRUE, incoming);
    case MOVE_EFFECT_EVA_UP_2_MINIMIZE:
        return FairAI_StageUtility(ctx, trainer, target, targetBattler, STAT_EVASION, 2, TRUE, incoming);
    case MOVE_EFFECT_SP_DEF_UP_DOUBLE_ELECTRIC_POWER:
        return FairAI_StageUtility(ctx, trainer, target, targetBattler, STAT_SPECIAL_DEFENSE, 1, TRUE, incoming)
            + FairAI_BestDamage(ctx, trainer, target, SPLIT_SPECIAL) / 4;
    case MOVE_EFFECT_STATUS_CONFUSE:
    case MOVE_EFFECT_CONFUSE_ALL_ADJACENT:
        if (liveTarget != NULL && (liveTarget->condition2 & STATUS2_CONFUSION)) {
            return FAIR_AI_INVALID;
        }
        return (incoming / 3 + target->hpPercent / 12) * accuracy / 100 - incoming;
    case MOVE_EFFECT_ATK_UP_2_STATUS_CONFUSION:
        if (liveTarget != NULL && (liveTarget->condition2 & STATUS2_CONFUSION)) {
            return FAIR_AI_INVALID;
        }
        return (incoming / 3 - FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL) / 2) * accuracy / 100 - incoming;
    case MOVE_EFFECT_CRIT_UP_2:
        return bestDamage == 0 ? FAIR_AI_INVALID : bestDamage / 4 - incoming;
    case MOVE_EFFECT_RESET_STAT_CHANGES: {
        int targetBoosts = FairAI_TargetPositiveStages(ctx, targetBattler);
        int ownBoosts = FairAI_OwnPositiveStages(trainer);
        return targetBoosts <= ownBoosts ? FAIR_AI_INVALID : (targetBoosts - ownBoosts) * 8 - incoming;
    }
    case MOVE_EFFECT_FORCE_SWITCH: {
        int boosts = FairAI_TargetPositiveStages(ctx, targetBattler);
        return boosts == 0 ? FAIR_AI_INVALID : boosts * 10 - incoming;
    }
    case MOVE_EFFECT_DISABLE:
    case MOVE_EFFECT_TORMENT:
    case MOVE_EFFECT_DECREASE_LAST_MOVE_PP:
        return targetBattler >= 0 && ctx->waza_no_old[targetBattler] != MOVE_NONE
            ? FairAI_DamageToTrainer(ctx, target, trainer, ctx->waza_no_old[targetBattler]) / 2 - incoming
            : FAIR_AI_INVALID;
    case MOVE_EFFECT_NEXT_ATTACK_ALWAYS_HITS: {
        int i;
        int gain = 0;
        for (i = 0; i < MAX_MON_MOVES; i++) {
            u16 candidate = trainer->moves[i];
            int damage;
            if (candidate == MOVE_NONE || ctx->moveTbl[candidate].split == SPLIT_STATUS || ctx->moveTbl[candidate].accuracy == 0) {
                continue;
            }
            damage = FairAI_DamageToKnown(ctx, trainer, target, candidate);
            damage = damage * (100 - ctx->moveTbl[candidate].accuracy) / 100;
            if (damage > gain) {
                gain = damage;
            }
        }
        return gain == 0 ? FAIR_AI_INVALID : gain - incoming;
    }
    case MOVE_EFFECT_PREVENT_STATUS: {
        int i;
        int threat = 0;
        if (ctx->side_condition[ownSide] & SIDE_STATUS_SAFEGUARD) {
            return FAIR_AI_INVALID;
        }
        for (i = 0; i < target->moveCount; i++) {
            if (ctx->moveTbl[target->moves[i]].split == SPLIT_STATUS) {
                threat += target->moveWeights[i] / 32;
            }
        }
        return threat == 0 ? FAIR_AI_INVALID : threat - incoming;
    }
    case MOVE_EFFECT_SURVIVE_WITH_1_HP:
        return incoming < trainer->hp * 100 / (trainer->maxHp ? trainer->maxHp : 1)
            ? FAIR_AI_INVALID : bestDamage / 2;
    case MOVE_EFFECT_MAX_ATK_LOSE_HALF_MAX_HP:
        if (trainer->hp * 2 <= trainer->maxHp || trainer->stages[STAT_ATTACK] >= 12) {
            return FAIR_AI_INVALID;
        }
        return FairAI_BestDamage(ctx, trainer, target, SPLIT_PHYSICAL) - 50 - incoming;
    case MOVE_EFFECT_STATUS_NIGHTMARE:
        return !(target->condition & STATUS_SLEEP) ? FAIR_AI_INVALID : 25 - incoming;
    case MOVE_EFFECT_TRANSFER_STATUS:
        return trainer->condition == 0 || target->condition != 0 ? FAIR_AI_INVALID : 18 - incoming;
    case MOVE_EFFECT_GROUND_TRAP_USER_CONTINUOUS_HEAL:
        return (trainer->effectFlags & MOVE_EFFECT_FLAG_INGRAIN) ? FAIR_AI_INVALID : 18 - incoming;
    case MOVE_EFFECT_GIVE_GROUND_IMMUNITY:
        return FairAI_Threat(ctx, target, trainer, SPLIT_PHYSICAL) == 0 ? FAIR_AI_INVALID : 12 - incoming;
    case MOVE_EFFECT_ALL_FAINT_3_TURNS:
        return liveTarget != NULL && (liveTarget->effect_of_moves & MOVE_EFFECT_FLAG_PERISH_SONG)
            ? FAIR_AI_INVALID : (target->hpPercent > trainer->hp * 100 / (trainer->maxHp ? trainer->maxHp : 1) ? 14 : 5) - incoming;
    case MOVE_EFFECT_FLEE_FROM_WILD_BATTLE:
        return FAIR_AI_INVALID;
    default:
        break;
    }
    switch (move) {
    case MOVE_TAILWIND:
        if (ctx->tailwindCount[ownSide] != 0) {
            return FAIR_AI_INVALID;
        }
        return FairAI_TeamSupportValue(bsys, ctx, battlerId, trainer, trainerSlot, target, -1) - incoming;
    case MOVE_LIGHT_SCREEN:
        return (ctx->side_condition[ownSide] & SIDE_STATUS_LIGHT_SCREEN) ? FAIR_AI_INVALID
            : FairAI_TeamSupportValue(bsys, ctx, battlerId, trainer, trainerSlot, target, SPLIT_SPECIAL) - incoming;
    case MOVE_REFLECT:
        return (ctx->side_condition[ownSide] & SIDE_STATUS_REFLECT) ? FAIR_AI_INVALID
            : FairAI_TeamSupportValue(bsys, ctx, battlerId, trainer, trainerSlot, target, SPLIT_PHYSICAL) - incoming;
    case MOVE_SPIKES:
        return ctx->scw[targetSide].spikesLayers >= 3 ? FAIR_AI_INVALID : 12 - incoming;
    case MOVE_TOXIC_SPIKES:
        return ctx->scw[targetSide].toxicSpikesLayers >= 2 ? FAIR_AI_INVALID : 12 - incoming;
    case MOVE_STEALTH_ROCK:
        return (ctx->side_condition[targetSide] & SIDE_STATUS_STEALTH_ROCK) ? FAIR_AI_INVALID : 12 - incoming;
    case MOVE_STICKY_WEB:
        return (ctx->side_condition[targetSide] & SIDE_STATUS_STICKY_WEB) ? FAIR_AI_INVALID : 10 - incoming;
    case MOVE_SUBSTITUTE:
        return trainer->hp * 100 / (trainer->maxHp ? trainer->maxHp : 1) <= 25 ? FAIR_AI_INVALID : incoming / 2 - 25;
    default:
        return FAIR_AI_UNKNOWN;
    }
}

static int FairAI_MoveUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, FairAIMonModel *trainer, int trainerSlot, const FairAIKnownMon *target, int targetBattler, u16 move)
{
    const struct BattleMove *data = &ctx->moveTbl[move];
    int incoming = FairAI_Threat(ctx, target, trainer, -1);

    if (data->split == SPLIT_STATUS) {
        return FairAI_StatusUtility(bsys, ctx, battlerId, trainer, trainerSlot, target, targetBattler, move, incoming);
    } else {
        int damage = FairAI_DamageToKnown(ctx, trainer, target, move);
        int targetHp = target != NULL ? target->hpPercent : 100;
        int firstChance = 50;
        int playerMove = MOVE_NONE;
        int i;

        if (damage <= FAIR_AI_UNKNOWN) {
            return damage;
        }
        if (target != NULL) {
            for (i = 0; i < target->moveCount; i++) {
                if (ctx->moveTbl[target->moves[i]].split != SPLIT_STATUS) {
                    playerMove = target->moves[i];
                    break;
                }
            }
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
        return damage - incoming;
    }
}

static BOOL FairAI_IsSamePartyDouble(struct BattleSystem *bsys)
{
    u32 battleType = BattleTypeGet(bsys);
    return (battleType & BATTLE_TYPE_DOUBLES) && !(battleType & (BATTLE_TYPE_MULTI | BATTLE_TYPE_TAG));
}

static BOOL FairAI_IsLegalReserve(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, int slot)
{
    struct PartyPokemon *mon = BattleWorkPokemonParamGet(bsys, battlerId, slot);

    if (mon == NULL || GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG, NULL)
        || GetMonData(mon, MON_DATA_HP, NULL) == 0 || slot == ctx->sel_mons_no[battlerId]
        || slot == ctx->ai_reshuffle_sel_mons_no[battlerId]) {
        return FALSE;
    }
    if (FairAI_IsSamePartyDouble(bsys)) {
        int ally = BATTLER_ALLY(battlerId);
        if (slot == ctx->sel_mons_no[ally] || slot == ctx->ai_reshuffle_sel_mons_no[ally]) {
            return FALSE;
        }
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

static const FairAIKnownMon *FairAI_FindPlayerReserve(struct BattleSystem *bsys, struct BattleStruct *ctx, int playerBattler)
{
    const FairAIKnownMon *best = NULL;
    int activeSlot = ctx->sel_mons_no[playerBattler];
    int i;

    for (i = 0; i < FAIR_AI_PARTY_SIZE; i++) {
        const FairAIKnownMon *candidate = &sFairAIState.playerMons[i];
        if (i == activeSlot || candidate->species == SPECIES_NONE || candidate->hpPercent == 0) {
            continue;
        }
        if (FairAI_IsSamePartyDouble(bsys) && i == ctx->sel_mons_no[BATTLER_ALLY(playerBattler)]) {
            continue;
        }
        if (best == NULL || candidate->hpPercent > best->hpPercent) {
            best = candidate;
        }
    }
    return best;
}

static int FairAI_PlayerSwitchChance(const FairAIKnownMon *active, const FairAIKnownMon *reserve, int trainerDamage)
{
    int chance = reserve == NULL ? 5 : 15;

    if (active != NULL && active->hpPercent <= 50) {
        chance += 20;
    }
    if (active != NULL && trainerDamage >= active->hpPercent) {
        chance += 25;
    }
    return chance > 70 ? 70 : chance;
}

static int FairAI_BestNextActionUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, FairAIMonModel *model, int trainerSlot, const FairAIKnownMon *target, int targetBattler)
{
    int best = FAIR_AI_INVALID;
    BOOL unknown = FALSE;
    int i;

    for (i = 0; i < MAX_MON_MOVES; i++) {
        int utility;
        u16 move = model->moves[i];

        if (move == MOVE_NONE || model->pp[i] == 0) {
            continue;
        }
        utility = FairAI_MoveUtility(bsys, ctx, battlerId, model, trainerSlot, target, targetBattler, move);
        if (utility == FAIR_AI_UNKNOWN) {
            unknown = TRUE;
        } else if (utility > best) {
            best = utility;
        }
    }
    if (best > FAIR_AI_INVALID) {
        return best;
    }
    return unknown ? FAIR_AI_UNKNOWN : FAIR_AI_INVALID;
}

static int FairAI_SwitchUtility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, int slot, const FairAIKnownMon *target, int targetBattler)
{
    FairAIMonModel reserve;
    struct PartyPokemon *partyMon = BattleWorkPokemonParamGet(bsys, battlerId, slot);
    int entryDamage;
    int nextAction;
    int hpPercent;
    int utility;

    FairAI_FillReserveModel(partyMon, &reserve);
    entryDamage = FairAI_Threat(ctx, target, &reserve, -1);
    hpPercent = reserve.hp * 100 / (reserve.maxHp ? reserve.maxHp : 1);
    utility = -entryDamage;

    // A switch consumes this turn. If the candidate is expected to survive
    // entry, value its best contextual action on the following turn. This is
    // deliberately not damage-only: screens, speed control, status, recovery,
    // and other support can make a reserve the correct reusable pivot.
    if (entryDamage < hpPercent) {
        nextAction = FairAI_BestNextActionUtility(bsys, ctx, battlerId, &reserve, slot, target, targetBattler);
        if (nextAction == FAIR_AI_UNKNOWN) {
            return FAIR_AI_UNKNOWN;
        }
        if (nextAction > FAIR_AI_INVALID) {
            utility += nextAction * FAIR_AI_FUTURE_PERCENT / 100;
        }
    }
    if (slot == sFairAIState.lastSwitchedOut[battlerId]
        && ctx->total_turn - sFairAIState.lastSwitchTurn[battlerId] <= FAIR_AI_RECENT_SWITCH_TURNS) {
        utility -= 20;
    }
    return utility;
}

static void FairAI_AddScenarioWeight(FairAIAction *actions, int count, int scenarioWeight, BOOL switched)
{
    int best = FAIR_AI_INVALID;
    int eligible = 0;
    int i;

    for (i = 0; i < count; i++) {
        int utility = switched ? actions[i].switchUtility : actions[i].stayUtility;
        if (utility > best && utility != FAIR_AI_UNKNOWN) {
            best = utility;
        }
    }
    for (i = 0; i < count; i++) {
        int utility = switched ? actions[i].switchUtility : actions[i].stayUtility;
        if (utility != FAIR_AI_UNKNOWN && utility > FAIR_AI_INVALID && utility >= best - FAIR_AI_NEAR_BEST_MARGIN) {
            eligible++;
        }
    }
    if (eligible == 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        int utility = switched ? actions[i].switchUtility : actions[i].stayUtility;
        if (utility != FAIR_AI_UNKNOWN && utility > FAIR_AI_INVALID && utility >= best - FAIR_AI_NEAR_BEST_MARGIN) {
            actions[i].weight += scenarioWeight / eligible;
        }
    }
}

static void FairAI_Decide(struct BattleSystem *bsys, int battlerId)
{
    struct BattleStruct *ctx = bsys->sp;
    FairAIDecision *decision = &sFairAIState.decisions[battlerId];
    FairAIMonModel trainer;
    FairAIAction actions[FAIR_AI_MAX_ACTIONS];
    const FairAIKnownMon *target;
    const FairAIKnownMon *playerReserve;
    u32 unusable;
    int targetBattler;
    int actionCount = 0;
    int unknown = 0;
    int switchChance;
    int totalWeight = 0;
    int roll;
    int i;
    u8 activeSlot = ctx->sel_mons_no[battlerId];

    if (sFairAIState.observedActiveSlot[battlerId] != activeSlot) {
        sFairAIState.observedActiveSlot[battlerId] = activeSlot;
        sFairAIState.actedSinceEntry[battlerId] = FALSE;
    }
    if (decision->valid && decision->turn == ctx->total_turn && decision->activeSlot == activeSlot) {
        return;
    }
    memset(decision, 0, sizeof(*decision));
    decision->turn = ctx->total_turn;
    decision->activeSlot = activeSlot;
    decision->command = 1;
    decision->move = 0xFF;
    decision->switchSlot = FAIR_AI_PARTY_SIZE;
    FairAI_RefreshKnowledge(bsys, ctx);
    FairAI_FillActiveModel(ctx, battlerId, &trainer);
    targetBattler = FairAI_GetTarget(bsys, ctx, battlerId);
    target = FairAI_GetRecord(ctx, targetBattler);
    playerReserve = FairAI_FindPlayerReserve(bsys, ctx, targetBattler);
    unusable = StruggleCheck(bsys, ctx, battlerId, 0, -1);

    for (i = 0; i < MAX_MON_MOVES; i++) {
        int stay;
        int switched;
        u16 move = trainer.moves[i];

        if (move == MOVE_NONE || trainer.pp[i] == 0 || (unusable & (1 << i))) {
            continue;
        }
        stay = FairAI_MoveUtility(bsys, ctx, battlerId, &trainer, ctx->sel_mons_no[battlerId], target, targetBattler, move);
        switched = playerReserve == NULL ? stay
                                         : FairAI_MoveUtility(bsys, ctx, battlerId, &trainer, ctx->sel_mons_no[battlerId], playerReserve, -1, move);
        if (stay == FAIR_AI_UNKNOWN || switched == FAIR_AI_UNKNOWN) {
            unknown = 1;
        }
        actions[actionCount].kind = FAIR_AI_ACTION_MOVE;
        actions[actionCount].index = i;
        actions[actionCount].weight = 0;
        actions[actionCount].stayUtility = stay;
        actions[actionCount].switchUtility = switched;
        actionCount++;
    }
    // A newly entered Pokemon must spend one command advancing the battle
    // before another voluntary switch. This keeps forced and voluntary entry
    // choices from becoming chains of zero-progress switch turns. If it has no
    // usable move, retain switching as an emergency legality fallback.
    if ((sFairAIState.actedSinceEntry[battlerId] || actionCount == 0)
        && CanSwitchMon(bsys, ctx, battlerId) && !FairAI_IsTrapped(bsys, ctx, battlerId)) {
        int partyCount = BattleWorkPokeCountGet(bsys, battlerId);
        for (i = 0; i < partyCount && i < FAIR_AI_PARTY_SIZE; i++) {
            if (!FairAI_IsLegalReserve(bsys, ctx, battlerId, i)) {
                continue;
            }
            actions[actionCount].kind = FAIR_AI_ACTION_SWITCH;
            actions[actionCount].index = i;
            actions[actionCount].weight = 0;
            actions[actionCount].stayUtility = FairAI_SwitchUtility(bsys, ctx, battlerId, i, target, targetBattler);
            actions[actionCount].switchUtility = playerReserve == NULL ? actions[actionCount].stayUtility
                : FairAI_SwitchUtility(bsys, ctx, battlerId, i, playerReserve, -1);
            actionCount++;
        }
    }
    if (actionCount == 0) {
        decision->valid = TRUE;
        return;
    }

    // An unforeseen effect falls back to a fair choice among all legal actions;
    // it is never silently discarded or assigned invented strategic value.
    if (unknown) {
        i = BattleRand(bsys) % actionCount;
    } else {
        int bestDamage = FairAI_BestDamage(ctx, &trainer, target, -1);
        switchChance = FairAI_PlayerSwitchChance(target, playerReserve, bestDamage);
        FairAI_AddScenarioWeight(actions, actionCount, 100 - switchChance, FALSE);
        FairAI_AddScenarioWeight(actions, actionCount, switchChance, TRUE);
        for (i = 0; i < actionCount; i++) {
            totalWeight += actions[i].weight;
        }
        if (totalWeight == 0) {
            i = BattleRand(bsys) % actionCount;
        } else {
            roll = BattleRand(bsys) % totalWeight;
            for (i = 0; i < actionCount; i++) {
                int weight = actions[i].weight;
                if (roll < weight) {
                    break;
                }
                roll -= weight;
            }
            if (i == actionCount) {
                i = 0;
            }
        }
    }
    decision->valid = TRUE;
    decision->target = targetBattler;
    if (actions[i].kind == FAIR_AI_ACTION_SWITCH) {
        decision->command = 3;
        decision->switchSlot = actions[i].index & 0xFF;
        sFairAIState.lastSwitchedOut[battlerId] = ctx->sel_mons_no[battlerId];
        sFairAIState.lastSwitchTurn[battlerId] = ctx->total_turn;
        ctx->ai_reshuffle_sel_mons_no[battlerId] = decision->switchSlot;
    } else {
        decision->move = actions[i].index & 0xFF;
        sFairAIState.actedSinceEntry[battlerId] = TRUE;
        ctx->aiWorkTable.ai_dir_select_client[battlerId] = targetBattler;
    }
}

u8 FairTrainerAI_PickCommand(struct BattleSystem *bsys, u8 battlerId)
{
    if (!TrainerAI_UsesStrategicLayer(bsys->trainers[battlerId].aibit)) {
        return TrainerAI_PickCommand_Original(bsys, battlerId);
    }
    FairAI_Decide(bsys, battlerId);
    return sFairAIState.decisions[battlerId].command;
}

u8 FairTrainerAI_PickMove(struct BattleSystem *bsys, u8 battlerId)
{
    if (!TrainerAI_UsesStrategicLayer(bsys->trainers[battlerId].aibit)) {
        return TrainerAI_PickMove_Original(bsys, battlerId);
    }
    FairAI_Decide(bsys, battlerId);
    return sFairAIState.decisions[battlerId].move;
}

#endif // IMPLEMENT_FAIR_TRAINER_AI
