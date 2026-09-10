#include "../include/legendary_sanctuary_internal.h"

#include "../include/config.h"
#include "../include/permanent_death.h"
#include "../include/pokedex.h"
#include "../include/pokemon_storage_system.h"
#include "../include/save.h"
#include "../include/task.h"
#include "../include/constants/maps.h"
#include "../include/constants/save.h"

#ifdef IMPLEMENT_LEGENDARY_SANCTUARY

#define SANCTUARY_PARTY_SIZE 6
#define SANCTUARY_EXIT_X 25
#define SANCTUARY_EXIT_Z 2
#define SANCTUARY_EXIT_DIRECTION 1

// Verified against the original Contest scripts. These identifiers do not
// have C constants in this repository's reduced field headers.
#define SANCTUARY_VAR_GATE_STATE 0x4118
#define SANCTUARY_VAR_ACTIVE_STATE 0x40F7
#define SANCTUARY_FLAG_OTHER_POKES_HELD 351
#define SANCTUARY_FLAG_TRANSITION 590
#define SANCTUARY_FLAG_ACTIVE 2451
#define SANCTUARY_FLAG_PARK_STATE 2454

void LONG_CALL sub_02093070(FieldSystem *fieldSystem);
void LONG_CALL Blackout_StartDeathWarp(
    TaskManager *taskManager,
    const Location *location);

static void Sanctuary_StoreFinalCandidate(SanctuaryWork *work)
{
    if ((work->state & SANCTUARY_CAUGHT_POKEMON) == 0) {
        return;
    }

    if (PokeParty_GetPokeCount(work->currentParty) < SANCTUARY_PARTY_SIZE) {
        if (!PokeParty_Add(work->currentParty, work->caughtPokemon)) {
            return;
        }
    } else {
        PCStorage *storage = SaveArray_Get(work->saveData, SAVE_ARRAY_PC_STORAGE);
        if (!PCStorage_PlaceMonInFirstEmptySlotInAnyBox(
                storage,
                &work->caughtPokemon->box)) {
            return;
        }
    }

    UpdatePokedexWithReceivedSpecies(work->saveData, work->caughtPokemon);
}

static BOOL Sanctuary_Delete(FieldSystem *fieldSystem)
{
    SanctuaryWork *work = Sanctuary_GetWork(fieldSystem);

    if (work == NULL) {
        return FALSE;
    }

    // Storage failure releases the candidate. It must never preserve a live
    // Contest session and strand the player in the Sanctuary lifecycle.
    Sanctuary_StoreFinalCandidate(work);
    sys_FreeMemoryEz(work->caughtPokemon);
    work->magic = 0;
    sys_FreeMemoryEz(work);
    fieldSystem->bugContest = NULL;
    return TRUE;
}

static void Sanctuary_ClearLifecycleState(FieldSystem *fieldSystem)
{
    SCRIPT_STATE *state = SavArray_Flags_get(fieldSystem->savedata);

    SetScriptVarPassSave(state, SANCTUARY_VAR_GATE_STATE, 2);
    SetScriptVarPassSave(state, SANCTUARY_VAR_ACTIVE_STATE, 0);
    ClearScriptFlagPassSave(state, SANCTUARY_FLAG_OTHER_POKES_HELD);
    ClearScriptFlagPassSave(state, SANCTUARY_FLAG_TRANSITION);
    ClearScriptFlagPassSave(state, SANCTUARY_FLAG_ACTIVE);
    ClearScriptFlagPassSave(state, SANCTUARY_FLAG_PARK_STATE);
}

void LONG_CALL Sanctuary_EndSession(FieldSystem *fieldSystem)
{
    if (!Sanctuary_Delete(fieldSystem)) {
        Sanctuary_ClearLifecycleState(fieldSystem);
        return;
    }

    Sanctuary_ClearLifecycleState(fieldSystem);
#ifdef IMPLEMENT_PERMANENT_DEATH
    if (!PermanentDeath_HasPendingNotification()) {
        PermanentDeath_ProcessPartyAfterBattle(fieldSystem->savedata);
    }
#endif
    sub_02093070(fieldSystem);
#ifdef IMPLEMENT_PERMANENT_DEATH
    PermanentDeath_ScheduleNotification(fieldSystem);
#endif
}

void LONG_CALL Sanctuary_WarpToExit(TaskManager *taskManager, FieldSystem *fieldSystem)
{
    const Location exit = {
        MAP_R35R0201,
        -1,
        SANCTUARY_EXIT_X,
        SANCTUARY_EXIT_Z,
        SANCTUARY_EXIT_DIRECTION,
    };

    Sanctuary_EndSession(fieldSystem);
    Blackout_StartDeathWarp(taskManager, &exit);
}

#endif
