#include "../include/level_cap.h"

#include "../include/battle.h"
#include "../include/battle_controller_player.h"
#include "../include/constants/trainer_id.h"
#include "../include/save.h"
#include "../include/script.h"

#ifdef IMPLEMENT_LEVEL_CAP

#define BATTLE_RESULT_SCRIPT_VAR 0x4013

typedef struct LevelCapReward {
    u16 trainerId;
    u8 newCap;
} LevelCapReward;

typedef struct EncounterTaskData {
    u32 *resultOut;
    s32 transitionEffect;
    s32 battleMusic;
    u32 unused_0C;
    BattleSetup *setup;
} EncounterTaskData;

typedef char EncounterTaskDataSetupOffsetCheck[(offsetof(EncounterTaskData, setup) == 0x10) ? 1 : -1];
typedef char BattleSetupWinFlagOffsetCheck[(offsetof(BattleSetup, winFlag) == 0x14) ? 1 : -1];
typedef char BattleSetupTrainerIdOffsetCheck[(offsetof(BattleSetup, trainerId) == 0x18) ? 1 : -1];

static const LevelCapReward sLevelCapRewards[] = {
    // Silver fight 1
    { TRAINER_ID_SILVER_1_CYNDAQUIL, 13 },
    { TRAINER_ID_SILVER_1_TOTODILE, 13 },
    { TRAINER_ID_SILVER_1_CHIKORITA, 13 },

    { TRAINER_ID_FALKNER, 17 },
    { TRAINER_ID_BUGSY, 18 },

    // Silver fight 2
    { TRAINER_ID_SILVER_2_BAYLEEF, 19 },
    { TRAINER_ID_SILVER_2_QUILAVA, 19 },
    { TRAINER_ID_SILVER_2_CROCONAW, 19 },

    { TRAINER_ID_WHITNEY, 22 },

    // Silver fight 3
    { TRAINER_ID_SILVER_3_BAYLEEF, 25 },
    { TRAINER_ID_SILVER_3_QUILAVA, 25 },
    { TRAINER_ID_SILVER_3_CROCONAW, 25 },

    { TRAINER_ID_MORTY, 31 },
    { TRAINER_ID_CHUCK, 35 },
    { TRAINER_ID_JASMINE, 35 },
    { TRAINER_ID_PRYCE, 35 },

    // Silver fight 4
    { TRAINER_ID_SILVER_4_MEGANIUM, 38 },
    { TRAINER_ID_SILVER_4_QUILAVA, 38 },
    { TRAINER_ID_SILVER_4_FERALIGATR, 38 },

    { TRAINER_ID_ARCHER, 41 },
    { TRAINER_ID_CLAIR, 41 },

    // Silver fight 5
    { TRAINER_ID_SILVER_5_MEGANIUM, 42 },
    { TRAINER_ID_SILVER_5_TYPHLOSION, 42 },
    { TRAINER_ID_SILVER_5_FERALIGATR, 42 },

    { TRAINER_ID_WILL, 44 },
    { TRAINER_ID_KOGA, 46 },
    { TRAINER_ID_BRUNO, 47 },
    { TRAINER_ID_KAREN, 50 },
    { TRAINER_ID_LANCE, 55 },

    // Each Kanto leader remains an independent balancing point.
    { TRAINER_ID_LT_SURGE, 55 },
    { TRAINER_ID_SABRINA, 55 },
    { TRAINER_ID_ERIKA, 55 },
    { TRAINER_ID_JANINE, 55 },
    { TRAINER_ID_MISTY, 55 },
    { TRAINER_ID_BROCK, 55 },

    // Silver fight 6
    { TRAINER_ID_SILVER_6_MEGANIUM, 59 },
    { TRAINER_ID_SILVER_6_TYPHLOSION, 59 },
    { TRAINER_ID_SILVER_6_FERALIGATR, 59 },

    { TRAINER_ID_BLAINE, 60 },
    { TRAINER_ID_BLUE, 86 },
    { TRAINER_ID_RED, 100 },
};

BOOL LONG_CALL SetNewLevelCap(u8 newCap)
{
    struct SAVE_MISC_DATA *misc = Sav2_Misc_get(SaveBlock2_get());

    if (misc->levelCap > newCap) {
        return FALSE;
    }

    misc->levelCap = newCap;
    return TRUE;
}

static u8 LevelCap_GetRewardForTrainer(u16 trainerId)
{
    u32 i;

    for (i = 0; i < NELEMS(sLevelCapRewards); i++) {
        if (sLevelCapRewards[i].trainerId == trainerId) {
            return sLevelCapRewards[i].newCap;
        }
    }

    return 0;
}

void LONG_CALL LevelCap_ApplyTrainerVictory(const BattleSetup *setup)
{
    u32 excludedBattleTypes = BATTLE_TYPE_WIRELESS | BATTLE_TYPE_BATTLE_TOWER | BATTLE_TYPE_DEBUG;
    u8 newCap;

    if (!(setup->battleType & BATTLE_TYPE_TRAINER) || (setup->battleType & excludedBattleTypes)) {
        return;
    }

    newCap = LevelCap_GetRewardForTrainer((u16)setup->trainerId[BATTLER_ENEMY]);
    if (newCap != 0) {
        SetNewLevelCap(newCap);
    }
}

BOOL LONG_CALL EncounterFinishLevelCapHook(EncounterTaskData *encounter, FieldSystem *fieldSystem)
{
    u32 outcome = encounter->setup->winFlag;

    if (encounter->resultOut != NULL) {
        *encounter->resultOut = outcome;
    }

    VarSet(fieldSystem, BATTLE_RESULT_SCRIPT_VAR, (u16)outcome);

    if (outcome == BATTLE_OUTCOME_WIN) {
        LevelCap_ApplyTrainerVictory(encounter->setup);
    }

    return outcome != BATTLE_OUTCOME_LOSE && outcome != BATTLE_OUTCOME_DRAW;
}

#endif // IMPLEMENT_LEVEL_CAP
