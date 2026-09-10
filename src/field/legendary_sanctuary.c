#include "../../include/legendary_sanctuary.h"
#include "../../include/legendary_sanctuary_internal.h"

#include "../../include/config.h"
#include "../../include/map_events.h"
#include "../../include/pokedex.h"
#include "../../include/pokemon.h"
#include "../../include/save.h"
#include "../../include/script.h"
#include "../../include/constants/file.h"
#include "../../include/constants/maps.h"
#include "../../include/constants/pokemon.h"
#include "../../include/constants/species.h"

#ifdef IMPLEMENT_LEGENDARY_SANCTUARY

#include "../../data/legendary_sanctuary_encounters.c"

#define SANCTUARY_ENGINE_TIME_LIMIT_MINUTES 20

_Static_assert(offsetof(SanctuaryWork, caughtPokemon) == 0x10,
    "Sanctuary candidate must preserve the Contest ABI");
_Static_assert(offsetof(SanctuaryWork, sanctuaryBalls) == 0x18,
    "Sanctuary Ball count must preserve the Contest ABI");
_Static_assert(offsetof(SanctuaryWork, magic) == SANCTUARY_ORIGINAL_WORK_SIZE,
    "Sanctuary extension must follow the original Contest work");

void LONG_CALL FieldSystem_StartBugContestTimer(FieldSystem *fieldSystem);
u32 LONG_CALL LCRandom(void);

static BOOL Sanctuary_IsEntryEligible(const SanctuaryWork *work, const SanctuaryEncounter *entry)
{
    if (entry->species == SPECIES_NONE || entry->minLevel == 0
        || entry->maxLevel < entry->minLevel) {
        return FALSE;
    }
    if (Pokedex_GetCaughtFlag(SaveData_GetDexPtr(work->saveData), entry->species)) {
        return FALSE;
    }
    return TRUE;
}

static BOOL Sanctuary_IsFirstEligiblePoolEntryForSpecies(const SanctuaryWork *work, u32 index)
{
    const SanctuaryEncounter *pool = gSanctuaryEncounterPools[work->stage];
    u32 i;

    for (i = 0; i < index; i++) {
        if (pool[i].species == pool[index].species
            && Sanctuary_IsEntryEligible(work, &pool[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

static u32 Sanctuary_CountEligibleEntries(const SanctuaryWork *work)
{
    u32 count = 0;
    u32 i;

    if (work->stage >= SANCTUARY_STAGE_COUNT) {
        return 0;
    }
    for (i = 0; i < SANCTUARY_MAX_POOL_SIZE; i++) {
        if (Sanctuary_IsEntryEligible(work, &gSanctuaryEncounterPools[work->stage][i])
            && Sanctuary_IsFirstEligiblePoolEntryForSpecies(work, i)) {
            count++;
        }
    }
    return count;
}

static BOOL Sanctuary_StageHasEligibleSpecies(FieldSystem *fieldSystem, u32 stage)
{
    struct Save_DexData *pokedex;
    u32 i;

    if (stage >= SANCTUARY_STAGE_COUNT) {
        return FALSE;
    }
    pokedex = SaveData_GetDexPtr(fieldSystem->savedata);
    for (i = 0; i < SANCTUARY_MAX_POOL_SIZE; i++) {
        const SanctuaryEncounter *entry = &gSanctuaryEncounterPools[stage][i];
        if (entry->species != SPECIES_NONE && entry->minLevel != 0
            && entry->maxLevel >= entry->minLevel
            && !Pokedex_GetCaughtFlag(pokedex, entry->species)) {
            return TRUE;
        }
    }
    return FALSE;
}

static u8 Sanctuary_FindLeadPokemon(struct Party *party)
{
    u8 i;

    for (i = 0; i < PokeParty_GetPokeCount(party); i++) {
        struct PartyPokemon *pokemon = Party_GetMonByIndex(party, i);
        if (GetMonData(pokemon, MON_DATA_HP, NULL) != 0
            && !GetMonData(pokemon, MON_DATA_IS_EGG, NULL)) {
            return i;
        }
    }
    return 0;
}

static SanctuaryWork *Sanctuary_New(FieldSystem *fieldSystem, u32 stage)
{
    // The original Contest work is allocated on main heap 3 because it must
    // survive the gate/interior field transition for the entire visit.
    SanctuaryWork *work = sys_AllocMemory(HEAPID_MAIN_HEAP, sizeof(*work));

    if (work == NULL) {
        return NULL;
    }
    memset(work, 0, sizeof(*work));
    work->heapId = HEAPID_MAIN_HEAP;
    work->saveData = fieldSystem->savedata;
    work->currentParty = SaveData_GetPlayerPartyPtr(fieldSystem->savedata);
    work->caughtPokemon = AllocMonZeroed(HEAPID_MAIN_HEAP);
    if (work->caughtPokemon == NULL) {
        sys_FreeMemoryEz(work);
        return NULL;
    }
    work->originalPartyCount = PokeParty_GetPokeCount(work->currentParty);
    work->leadPokemonIndex = Sanctuary_FindLeadPokemon(work->currentParty);
    work->sanctuaryBalls = SANCTUARY_BALL_COUNT;
    // The original field lifecycle expires at 20 elapsed minutes and its
    // formatter displays 20 - elapsed. Offsetting the initial value preserves
    // both proven paths while exposing a shorter minute-based duration.
    work->elapsedMinutes =
        SANCTUARY_ENGINE_TIME_LIMIT_MINUTES - SANCTUARY_TIME_LIMIT_MINUTES;
    work->magic = SANCTUARY_WORK_MAGIC;
    work->stage = stage < SANCTUARY_STAGE_COUNT ? stage : 0;
    FieldSystem_StartBugContestTimer(fieldSystem);
    return work;
}

BOOL LONG_CALL Sanctuary_ScrCmdAction(SCRIPTCONTEXT *ctx)
{
    u8 action = ScriptReadByte(ctx);
    u32 stage = ScriptGetVar(ctx);
    FieldSystem *fieldSystem = ctx->fsys;
    u16 *result = FieldSysGetAttrAddr(fieldSystem, SCRIPTENV_SPECIAL_VAR_RESULT);

    if (action == 2) {
        *result = Sanctuary_StageHasEligibleSpecies(fieldSystem, stage);
        return FALSE;
    }

    if (action == 0) {
        *result = FALSE;
        if (fieldSystem->bugContest == NULL
            && Sanctuary_StageHasEligibleSpecies(fieldSystem, stage)) {
            fieldSystem->bugContest = Sanctuary_New(fieldSystem, stage);
        }
        *result = fieldSystem->bugContest != NULL;
        return FALSE;
    }

    Sanctuary_EndSession(fieldSystem);
    *result = TRUE;
    return FALSE;
}

u32 LONG_CALL Sanctuary_GetWalkingEncounterRate(FieldSystem *fieldSystem)
{
    const EncounterData *encounterData = MapEvents_GetLoadedEncTable(fieldSystem);
    SanctuaryWork *work;

    if (fieldSystem->location->mapId != MAP_D22R0102) {
        return encounterData->rateWalk;
    }
    work = Sanctuary_GetWork(fieldSystem);
    return work != NULL && Sanctuary_CountEligibleEntries(work) != 0
        ? SANCTUARY_ENCOUNTER_RATE
        : 0;
}

SanctuaryEncounterSlot *LONG_CALL Sanctuary_GetEncounterSlot(void *session, u32 heapId)
{
    SanctuaryWork *work = session;
    SanctuaryEncounterSlot *slot;
    u32 eligibleCount;
    u32 selected;
    u32 i;

    if (work == NULL || work->magic != SANCTUARY_WORK_MAGIC
        || (eligibleCount = Sanctuary_CountEligibleEntries(work)) == 0) {
        return NULL;
    }

    selected = LCRandom() % eligibleCount;
    for (i = 0; i < SANCTUARY_MAX_POOL_SIZE; i++) {
        const SanctuaryEncounter *entry = &gSanctuaryEncounterPools[work->stage][i];
        if (!Sanctuary_IsEntryEligible(work, entry)
            || !Sanctuary_IsFirstEligiblePoolEntryForSpecies(work, i)) {
            continue;
        }
        if (selected-- == 0) {
            u32 levelRange = entry->maxLevel - entry->minLevel + 1;
            slot = sys_AllocMemoryLo(heapId, sizeof(*slot));
            if (slot == NULL) {
                return NULL;
            }
            slot->species = entry->species;
            slot->maxLevel = entry->minLevel + (LCRandom() % levelRange);
            slot->minLevel = 0;
            return slot;
        }
    }
    return NULL;
}

#endif
