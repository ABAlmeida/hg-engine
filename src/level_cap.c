#include "../include/level_cap.h"

#include "../include/battle.h"
#include "../include/battle_controller_player.h"
#include "../include/constants/file.h"
#include "../include/save.h"
#include "../include/script.h"

#ifdef IMPLEMENT_LEVEL_CAP

#define BATTLE_RESULT_SCRIPT_VAR 0x4013

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
    u8 newCap = 0;

    if (trainerId >= LEVEL_CAP_REWARD_TABLE_COUNT) {
        return 0;
    }

    ReadFromNarcMemberByIdPair(&newCap, ARC_CODE_ADDONS, CODE_ADDON_LEVEL_CAP_REWARDS, trainerId, sizeof(newCap));
    return newCap;
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
