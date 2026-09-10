#ifndef LEGENDARY_SANCTUARY_INTERNAL_H
#define LEGENDARY_SANCTUARY_INTERNAL_H

#include "legendary_sanctuary.h"
#include "pokemon.h"

#define SANCTUARY_WORK_MAGIC 0x53414E43
#define SANCTUARY_CAUGHT_POKEMON (1 << 0)
#define SANCTUARY_ORIGINAL_WORK_SIZE 0xC4

typedef struct SanctuaryWork {
    u32 heapId;
    struct SaveData *saveData;
    struct Party *legacyPartyBackup;
    struct Party *currentParty;
    struct PartyPokemon *caughtPokemon;
    u8 leadPokemonIndex;
    u8 originalPartyCount;
    u8 legacyDayOfWeek;
    u8 state;
    u16 sanctuaryBalls;
    u16 legacyPrize;
    u32 elapsedMinutes;
    u8 legacyWork[SANCTUARY_ORIGINAL_WORK_SIZE - 0x20];

    u32 magic;
    u8 stage;
} SanctuaryWork;

static inline SanctuaryWork *Sanctuary_GetWork(FieldSystem *fieldSystem)
{
    SanctuaryWork *work = fieldSystem == NULL ? NULL : fieldSystem->bugContest;

    if (work == NULL || work->magic != SANCTUARY_WORK_MAGIC) {
        return NULL;
    }
    return work;
}

// This implementation is resident but is also called from the field
// extension, so the caller must preserve Thumb state across the long branch.
void LONG_CALL Sanctuary_EndSession(FieldSystem *fieldSystem);

#endif
