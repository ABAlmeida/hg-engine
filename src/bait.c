#include "../include/bait.h"

#include "../include/bag.h"
#include "../include/config.h"
#include "../include/constants/file.h"
#include "../include/constants/item.h"
#include "../include/constants/maps.h"
#include "../include/encounter_check.h"
#include "../include/item.h"
#include "../include/map_events.h"
#include "../include/metatile_behavior.h"
#include "../include/pokemon.h"
#include "../include/save.h"
#include "../include/task.h"

#define ITEM_USE_ERROR_NOT_NOW 3
#define ITEM_MENU_STATE_EXIT_TO_FIELD_TASK 12

#ifdef IMPLEMENT_BAIT_ENCOUNTERS

typedef enum BaitEncounterMode {
    BAIT_ENCOUNTER_MODE_NONE,
    BAIT_ENCOUNTER_MODE_NORMAL,
    BAIT_ENCOUNTER_MODE_SHINY,
} BaitEncounterMode;

typedef struct BaitEncounterTaskData {
    BaitEncounterMode mode;
} BaitEncounterTaskData;

typedef enum AlphPuzzleIndex {
    ALPH_PUZZLE_KABUTO,
    ALPH_PUZZLE_AERODACTYL,
    ALPH_PUZZLE_OMANYTE,
    ALPH_PUZZLE_HO_OH,
    ALPH_PUZZLE_COUNT,
} AlphPuzzleIndex;

static BaitEncounterMode sBaitEncounterMode;

static BOOL Bait_MapRequiresUnlockedAlphPuzzle(u32 mapId)
{
    switch (mapId) {
    case MAP_RUINS_OF_ALPH_UNDERGROUND_HALL:
    case MAP_RUINS_OF_ALPH_UNDERGROUND_HALL_SINJOH_EVENT:
    case MAP_RUINS_OF_ALPH_HALL_ENTRANCE_SINJOH_EVENT:
    case MAP_RUINS_OF_ALPH_UNDERGROUND_HALL_SINJOH_EVENT_2:
        return TRUE;

    default:
        return FALSE;
    }
}

static BOOL Bait_HasUnlockedAlphPuzzle(void *eventData)
{
    AlphPuzzleIndex puzzle;

    for (puzzle = ALPH_PUZZLE_KABUTO; puzzle < ALPH_PUZZLE_COUNT; puzzle++) {
        if (SaveVarsFlags_CheckAlphPuzzleFlag(eventData, puzzle)) {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL Bait_CanGenerateEncounter(const struct ItemCheckUseData *checkData, void *eventData)
{
    const EncounterData *encounterData = MapEvents_GetLoadedEncTable(checkData->fieldSystem);
    u8 metatileBehavior = checkData->standingTile;

    if (!MetatileBehavior_CanGenerateWalkingEncounters(metatileBehavior)) {
        return FALSE;
    }

    if (MetatileBehavior_IsSurfableWater(metatileBehavior)) {
        if (encounterData->rateSurf == 0) {
            return FALSE;
        }
    } else if (encounterData->rateWalk == 0) {
        return FALSE;
    }

    if (Bait_MapRequiresUnlockedAlphPuzzle(checkData->mapId)) {
        return Bait_HasUnlockedAlphPuzzle(eventData);
    }

    return TRUE;
}

static BOOL LONG_CALL Task_BaitEncounter(TaskManager *taskManager)
{
    BaitEncounterTaskData *data = taskManager->env;
    BOOL encounterStarted;

    sBaitEncounterMode = data->mode;
    encounterStarted = FieldSystem_PerformSweetScentEncounterCheck(taskManager->fieldSystem, taskManager);
    sBaitEncounterMode = BAIT_ENCOUNTER_MODE_NONE;
    sys_FreeMemoryEz(data);

    // A successful check replaces this task with the normal wild-battle task.
    return !encounterStarted;
}

u32 LONG_CALL ItemCheckUseFunc_Bait(const struct ItemCheckUseData *checkData)
{
    void *eventData = SaveData_GetEventPtr(checkData->fieldSystem->savedata);

    if (SaveVarsFlags_CheckSafariSysFlag(eventData) || SaveVarsFlags_CheckBugContestFlag(eventData)) {
        return ITEM_USE_ERROR_NOT_NOW;
    }

    if (!Bait_CanGenerateEncounter(checkData, eventData)) {
        return ITEM_USE_ERROR_NOT_NOW;
    }

    return 0;
}

void LONG_CALL ItemMenuUseFunc_Bait(struct ItemMenuUseData *data, const struct ItemCheckUseData *checkData UNUSED)
{
    FieldSystem *fieldSystem = data->taskManager->fieldSystem;
    struct BagViewAppWork *env = data->taskManager->env;
    BaitEncounterTaskData *taskData;

    FieldSystem_LoadFieldOverlay(fieldSystem);
    taskData = sys_AllocMemory(HEAPID_WORLD, sizeof(BaitEncounterTaskData));

    sBaitEncounterMode = BAIT_ENCOUNTER_MODE_NONE;
    taskData->mode = data->itemId == ITEM_SHINY_BAIT
        ? BAIT_ENCOUNTER_MODE_SHINY
        : BAIT_ENCOUNTER_MODE_NORMAL;

    env->atexit_TaskFunc = Task_BaitEncounter;
    env->atexit_TaskEnv = taskData;
    env->state = ITEM_MENU_STATE_EXIT_TO_FIELD_TASK;
    Bag_TakeItem(Sav2_Bag_get(fieldSystem->savedata), data->itemId, 1, HEAPID_WORLD);
}

BOOL LONG_CALL Bait_IsGeneratingEncounter(void)
{
    return sBaitEncounterMode != BAIT_ENCOUNTER_MODE_NONE;
}

BOOL LONG_CALL Bait_ShouldForceShiny(void)
{
    return sBaitEncounterMode == BAIT_ENCOUNTER_MODE_SHINY;
}

#else

u32 LONG_CALL ItemCheckUseFunc_Bait(const struct ItemCheckUseData *checkData UNUSED)
{
    return ITEM_USE_ERROR_NOT_NOW;
}

void LONG_CALL ItemMenuUseFunc_Bait(struct ItemMenuUseData *data UNUSED, const struct ItemCheckUseData *checkData UNUSED)
{
}

BOOL LONG_CALL Bait_IsGeneratingEncounter(void)
{
    return FALSE;
}

BOOL LONG_CALL Bait_ShouldForceShiny(void)
{
    return FALSE;
}

#endif
