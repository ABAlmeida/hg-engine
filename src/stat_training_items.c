#include "../include/stat_training_items.h"

#include "../include/config.h"
#include "../include/pokemon.h"
#include "../include/constants/item.h"
#include "../include/constants/species.h"

#define NUM_TRAINED_STATS            6
#define EV_AMOUNT_SMALL              10
#define EV_AMOUNT_STANDARD           50
#define EV_AMOUNT_LARGE              100
#define EV_LIMIT_PER_STAT            252
#define EV_LIMIT_TOTAL               510
#define IV_MAX_VALUE                 31
#define FRIENDSHIP_EVENT_VITAMIN     1
#define MON_MOOD_MODIFIER_VITAMIN    1

#ifdef UPDATE_VITAMIN_EV_CAPS

enum StatTrainingStat {
    STAT_TRAINING_HP,
    STAT_TRAINING_ATTACK,
    STAT_TRAINING_DEFENSE,
    STAT_TRAINING_SPEED,
    STAT_TRAINING_SP_ATTACK,
    STAT_TRAINING_SP_DEFENSE,
};

// Variant IDs deliberately follow the contiguous MON_DATA_* EV/IV stat order.
_Static_assert(ITEM_ZINC_S - ITEM_HP_UP_S + 1 == NUM_TRAINED_STATS,
    "Small vitamin IDs must contain all six stats");
_Static_assert(ITEM_ZINC_L - ITEM_HP_UP_L + 1 == NUM_TRAINED_STATS,
    "Large vitamin IDs must contain all six stats");
_Static_assert(ITEM_ZINC_MAX - ITEM_HP_UP_MAX + 1 == NUM_TRAINED_STATS,
    "Max vitamin IDs must contain all six stats");
_Static_assert(MON_DATA_SPDEF_EV - MON_DATA_HP_EV + 1 == NUM_TRAINED_STATS,
    "Pokemon EV fields must remain contiguous and in stat order");
_Static_assert(MON_DATA_SPDEF_IV - MON_DATA_HP_IV + 1 == NUM_TRAINED_STATS,
    "Pokemon IV fields must remain contiguous and in stat order");

static BOOL StatTrainingItem_GetVitaminEffect(u16 itemId, u8 *stat, u16 *amount)
{
    switch (itemId) {
    case ITEM_HP_UP:
        *stat = STAT_TRAINING_HP;
        break;
    case ITEM_PROTEIN:
        *stat = STAT_TRAINING_ATTACK;
        break;
    case ITEM_IRON:
        *stat = STAT_TRAINING_DEFENSE;
        break;
    case ITEM_CARBOS:
        *stat = STAT_TRAINING_SPEED;
        break;
    case ITEM_CALCIUM:
        *stat = STAT_TRAINING_SP_ATTACK;
        break;
    case ITEM_ZINC:
        *stat = STAT_TRAINING_SP_DEFENSE;
        break;
    default:
        if (itemId >= ITEM_HP_UP_S && itemId <= ITEM_ZINC_S) {
            *stat = itemId - ITEM_HP_UP_S;
            *amount = EV_AMOUNT_SMALL;
            return TRUE;
        }
        if (itemId >= ITEM_HP_UP_L && itemId <= ITEM_ZINC_L) {
            *stat = itemId - ITEM_HP_UP_L;
            *amount = EV_AMOUNT_LARGE;
            return TRUE;
        }
        if (itemId >= ITEM_HP_UP_MAX && itemId <= ITEM_ZINC_MAX) {
            *stat = itemId - ITEM_HP_UP_MAX;
            *amount = EV_LIMIT_PER_STAT;
            return TRUE;
        }
        return FALSE;
    }

    *amount = EV_AMOUNT_STANDARD;
    return TRUE;
}

BOOL StatTrainingItem_IsHandled(u16 itemId)
{
    u8 stat;
    u16 amount;

    return itemId == ITEM_IV_MAX
        || StatTrainingItem_GetVitaminEffect(itemId, &stat, &amount);
}

BOOL StatTrainingItem_CanUseOnMon(struct PartyPokemon *mon, u16 itemId)
{
    u8 stat;
    u16 amount;
    u32 total = 0;

    if (GetMonData(mon, MON_DATA_IS_EGG, NULL)) {
        return FALSE;
    }

    if (itemId == ITEM_IV_MAX) {
        for (stat = 0; stat < NUM_TRAINED_STATS; stat++) {
            if (GetMonData(mon, MON_DATA_HP_IV + stat, NULL) < IV_MAX_VALUE) {
                return TRUE;
            }
        }
        return FALSE;
    }

    if (!StatTrainingItem_GetVitaminEffect(itemId, &stat, &amount)
        || (stat == STAT_TRAINING_HP
            && GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_SHEDINJA)) {
        return FALSE;
    }

    for (u8 i = 0; i < NUM_TRAINED_STATS; i++) {
        total += GetMonData(mon, MON_DATA_HP_EV + i, NULL);
    }

    return total < EV_LIMIT_TOTAL
        && GetMonData(mon, MON_DATA_HP_EV + stat, NULL) < EV_LIMIT_PER_STAT;
}

BOOL LONG_CALL StatTrainingItem_UseOnMon(
    struct PartyPokemon *mon, u16 itemId, u16 mapSection)
{
    u8 stat;
    u16 amount;

    if (!StatTrainingItem_CanUseOnMon(mon, itemId)) {
        return FALSE;
    }

    if (itemId == ITEM_IV_MAX) {
        u8 iv = IV_MAX_VALUE;

        for (stat = 0; stat < NUM_TRAINED_STATS; stat++) {
            SetMonData(mon, MON_DATA_HP_IV + stat, &iv);
        }
    } else {
        u32 total = 0;
        u32 current;
        u32 available;
        u8 updated;

        StatTrainingItem_GetVitaminEffect(itemId, &stat, &amount);
        for (u8 i = 0; i < NUM_TRAINED_STATS; i++) {
            total += GetMonData(mon, MON_DATA_HP_EV + i, NULL);
        }

        current = GetMonData(mon, MON_DATA_HP_EV + stat, NULL);
        available = EV_LIMIT_PER_STAT - current;
        if (available > EV_LIMIT_TOTAL - total) {
            available = EV_LIMIT_TOTAL - total;
        }
        if (amount > available) {
            amount = available;
        }

        updated = current + amount;
        SetMonData(mon, MON_DATA_HP_EV + stat, &updated);
        MonApplyFriendshipMod(mon, FRIENDSHIP_EVENT_VITAMIN, mapSection);
        ApplyMonMoodModifier(mon, MON_MOOD_MODIFIER_VITAMIN);
    }

    RecalcPartyPokemonStats(mon);
    return TRUE;
}

#else

BOOL StatTrainingItem_IsHandled(u16 itemId UNUSED)
{
    return FALSE;
}

BOOL StatTrainingItem_CanUseOnMon(struct PartyPokemon *mon UNUSED, u16 itemId UNUSED)
{
    return FALSE;
}

BOOL LONG_CALL StatTrainingItem_UseOnMon(
    struct PartyPokemon *mon UNUSED, u16 itemId UNUSED, u16 mapSection UNUSED)
{
    return FALSE;
}

#endif
