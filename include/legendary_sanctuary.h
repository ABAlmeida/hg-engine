#ifndef LEGENDARY_SANCTUARY_H
#define LEGENDARY_SANCTUARY_H

#include "config.h"
#include "types.h"

typedef struct FieldSystem FieldSystem;
typedef struct SCRIPTCONTEXT SCRIPTCONTEXT;
typedef struct TaskManager TaskManager;
struct BATTLE_PARAM;
struct PartyPokemon;

typedef struct SanctuaryEncounter {
    u16 species;
    u8 minLevel;
    u8 maxLevel;
} SanctuaryEncounter;

typedef struct SanctuaryEncounterSlot {
    u32 species;
    u16 maxLevel;
    u16 minLevel;
} SanctuaryEncounterSlot;

extern const SanctuaryEncounter
    gSanctuaryEncounterPools[SANCTUARY_STAGE_COUNT][SANCTUARY_MAX_POOL_SIZE];

BOOL LONG_CALL Sanctuary_ScrCmdAction(SCRIPTCONTEXT *ctx);
void LONG_CALL Sanctuary_WarpToExit(TaskManager *taskManager, FieldSystem *fieldSystem);
u32 LONG_CALL Sanctuary_GetWalkingEncounterRate(FieldSystem *fieldSystem);
SanctuaryEncounterSlot *LONG_CALL Sanctuary_GetEncounterSlot(void *work, u32 heapId);
#endif
