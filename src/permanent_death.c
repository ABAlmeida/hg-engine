#include "../include/permanent_death.h"

#include "../include/battle.h"
#include "../include/config.h"
#include "../include/pokemon.h"
#include "../include/pokemon_storage_system.h"
#include "../include/save.h"
#include "../include/script.h"
#include "../include/task.h"
#include "../include/constants/file.h"

#ifdef IMPLEMENT_PERMANENT_DEATH

#define PERMANENT_DEATH_EXCLUDED_BATTLE_TYPES                                                    \
    (BATTLE_TYPE_WIRELESS | BATTLE_TYPE_SAFARI | BATTLE_TYPE_BATTLE_TOWER | BATTLE_TYPE_PAL_PARK \
        | BATTLE_TYPE_CATCHING_DEMO | BATTLE_TYPE_IMPORTED | BATTLE_TYPE_DEBUG)

enum {
    PERMANENT_DEATH_NO_PARTY_SLOT = 0xFF,
    PERMANENT_DEATH_BLACKOUT_MESSAGE = 7,
    PERMANENT_DEATH_NOTIFICATION_SCRIPT = 2074,
};

typedef struct BlackoutEnvironmentPrefix {
    u32 state;
    FieldSystem *fieldSystem;
} BlackoutEnvironmentPrefix;

_Static_assert(offsetof(struct BattleSetup, battleType) == 0, "BattleSetup battle-type offset changed");
_Static_assert(offsetof(FieldSystem, savedata) == 0xC, "FieldSystem save-data offset changed");
_Static_assert(
    offsetof(BlackoutEnvironmentPrefix, fieldSystem) == 4,
    "Blackout environment field-system offset changed");

extern void LONG_CALL BattleSetup_CommitToSave(
    struct BattleSetup *setup,
    FieldSystem *fieldSystem);
extern void LONG_CALL Blackout_PrintMessage(void *environment, s32 messageId, u8 x, u8 y);
extern void LONG_CALL Blackout_StartDeathWarp(TaskManager *taskManager, const void *deathWarp);
extern void LONG_CALL BeginNormalPaletteFade(
    int pattern,
    int typeTop,
    int typeBottom,
    u16 color,
    int duration,
    int framesPer,
    int heapId);
extern void LONG_CALL CallTask_RestoreOverworld(TaskManager *taskManager);
extern BOOL LONG_CALL FieldSystem_IsMainApplicationRunning(FieldSystem *fieldSystem);
extern BOOL LONG_CALL Task_WaitPaletteFade(TaskManager *taskManager);

// These values cross battle, field, and script lifetimes and therefore remain
// in overlay 129. Field-task ownership stays in the field extension itself.
static u8 sNotificationPending;
static u8 sPendingRecoveredPartySlot = PERMANENT_DEATH_NO_PARTY_SLOT;
static SysTask *sNotificationDispatchTask;

static BOOL PermanentDeath_PartyHasUsablePokemon(struct Party *party)
{
    int i;

    for (i = 0; i < PokeParty_GetPokeCount(party); i++) {
        struct PartyPokemon *mon = Party_GetMonByIndex(party, i);

        if (GetMonData(mon, MON_DATA_SPECIES_EXISTS, NULL)
            && !GetMonData(mon, MON_DATA_IS_EGG, NULL)) {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL PermanentDeath_TryRecoverFromStorage(SaveData *saveData, u8 *partySlot)
{
    struct Party *party = SaveData_GetPlayerPartyPtr(saveData);
    PCStorage *storage = SaveArray_Get(saveData, SAVE_ARRAY_PC_STORAGE);
    struct PartyPokemon recoveredMon;
    u32 box;
    u32 slot;

    for (box = 0; box < NUM_PC_BOXES; box++) {
        for (slot = 0; slot < MONS_PER_BOX; slot++) {
            struct BoxPokemon *boxMon = PCStorage_GetMonByIndexPair(storage, box, slot);

            if (!GetBoxMonData(boxMon, MON_DATA_SPECIES_EXISTS, NULL)
                || GetBoxMonData(boxMon, MON_DATA_IS_EGG, NULL)) {
                continue;
            }

            CopyBoxPokemonToPokemon(boxMon, &recoveredMon);

            // Keep the boxed copy unless conversion into a party member
            // succeeds, so an unexpected full-party state cannot lose it.
            if (!PokeParty_Add(party, &recoveredMon)) {
                return FALSE;
            }

            *partySlot = PokeParty_GetPokeCount(party) - 1;
            PCStorage_DeleteBoxMonByIndexPair(storage, box, slot);
            return TRUE;
        }
    }

    return FALSE;
}

void PermanentDeath_ProcessPartyAfterBattle(SaveData *saveData)
{
    struct Party *party = SaveData_GetPlayerPartyPtr(saveData);
    BOOL removedPokemon = FALSE;
    int i;

    // Work backwards because each deletion compacts the later party slots.
    for (i = PokeParty_GetPokeCount(party); i > 0; i--) {
        struct PartyPokemon *mon = Party_GetMonByIndex(party, i - 1);

        if (!GetMonData(mon, MON_DATA_SPECIES_EXISTS, NULL)
            || GetMonData(mon, MON_DATA_IS_EGG, NULL)
            || GetMonData(mon, MON_DATA_HP, NULL) != 0) {
            continue;
        }

        PokeParty_Delete(party, i - 1);
        removedPokemon = TRUE;
    }

    sNotificationPending = removedPokemon;
    sPendingRecoveredPartySlot = PERMANENT_DEATH_NO_PARTY_SLOT;

    if (removedPokemon && !PermanentDeath_PartyHasUsablePokemon(party)) {
        PermanentDeath_TryRecoverFromStorage(saveData, &sPendingRecoveredPartySlot);
    }
}

BOOL PermanentDeath_HasPendingNotification(void)
{
    return sNotificationPending;
}

u8 PermanentDeath_GetRecoveredPartySlot(void)
{
    return sPendingRecoveredPartySlot;
}

BOOL PermanentDeath_ShouldEndRun(SaveData *saveData)
{
    // Eggs may remain after a wipe but cannot provide a usable battle state.
    return sNotificationPending
        && sPendingRecoveredPartySlot == PERMANENT_DEATH_NO_PARTY_SLOT
        && !PermanentDeath_PartyHasUsablePokemon(SaveData_GetPlayerPartyPtr(saveData));
}

static void PermanentDeath_TaskWaitForFieldIdle(SysTask *task, void *data)
{
    FieldSystem *fieldSystem = data;

    if (!PermanentDeath_HasPendingNotification()) {
        sNotificationDispatchTask = NULL;
        DestroySysTask(task);
        return;
    }

    // A trainer or scripted encounter still owns the field TaskManager after
    // the battle fade. Wait until that script has finished setting flags and
    // releasing the field before starting a new map-scene script.
    if (fieldSystem->taskman != NULL
        || !FieldSystem_IsMainApplicationRunning(fieldSystem)) {
        return;
    }

    sNotificationDispatchTask = NULL;
    DestroySysTask(task);
    EventSet_Script(fieldSystem, PERMANENT_DEATH_NOTIFICATION_SCRIPT, NULL);
}

void PermanentDeath_ScheduleNotification(FieldSystem *fieldSystem)
{
    if (!PermanentDeath_HasPendingNotification()
        || sNotificationDispatchTask != NULL) {
        return;
    }

    sNotificationDispatchTask =
        CreateSysTask(PermanentDeath_TaskWaitForFieldIdle, fieldSystem, 0);
    GF_ASSERT(sNotificationDispatchTask != NULL);
}

void LONG_CALL PermanentDeath_BlackoutWarpOrSkip(
    TaskManager *taskManager,
    const void *deathWarp)
{
    if (!PermanentDeath_ShouldEndRun(taskManager->fieldSystem->savedata)) {
        Blackout_StartDeathWarp(taskManager, deathWarp);
    }
}

void LONG_CALL PermanentDeath_BlackoutPrintMessage(
    void *environment,
    s32 messageId,
    u8 x,
    u8 y)
{
    BlackoutEnvironmentPrefix *blackout = environment;

    if (PermanentDeath_ShouldEndRun(blackout->fieldSystem->savedata)) {
        messageId = PERMANENT_DEATH_BLACKOUT_MESSAGE;
    }

    Blackout_PrintMessage(environment, messageId, x, y);
}

void LONG_CALL PermanentDeath_BlackoutRestoreOrEnd(TaskManager *taskManager)
{
    FieldSystem *fieldSystem = taskManager->fieldSystem;

    if (!PermanentDeath_ShouldEndRun(fieldSystem->savedata)) {
        CallTask_RestoreOverworld(taskManager);
        return;
    }

    // The blackout message has already closed and faded out. Reset directly
    // instead of returning into the blackout and parent-script task stacks.
    PermanentDeath_FinishNotification(TRUE);
}

static BOOL PermanentDeath_TaskScheduleNotificationAfterFade(TaskManager *taskManager)
{
    if (taskManager->state == 0) {
        TaskManager_Call(taskManager, Task_WaitPaletteFade, NULL);
        taskManager->state++;
        return FALSE;
    }

    PermanentDeath_ScheduleNotification(taskManager->fieldSystem);
    return TRUE;
}

void LONG_CALL PermanentDeath_CallTaskFadeFromBlack(TaskManager *taskManager)
{
    FieldSystem *fieldSystem = taskManager->fieldSystem;

    // This ARM9 hook executes before overlay 131 is guaranteed to be loaded.
    // Its entry point and deferred callback must therefore remain resident.
    if (!FieldSystem_IsMainApplicationRunning(fieldSystem)) {
        GF_ASSERT(0);
        return;
    }

    BeginNormalPaletteFade(0, 1, 1, 0, 6, 1, HEAPID_FIELD1);
    TaskManager_Call(
        taskManager,
        PermanentDeath_HasPendingNotification()
            ? PermanentDeath_TaskScheduleNotificationAfterFade
            : Task_WaitPaletteFade,
        NULL);
}

void PermanentDeath_FinishNotification(BOOL endRun)
{
    sNotificationPending = FALSE;
    sPendingRecoveredPartySlot = PERMANENT_DEATH_NO_PARTY_SLOT;

    if (endRun) {
        // A system reset is safe from an active HGSS field command and avoids
        // depending on the current script, task, and overlays to unwind.
        OS_ResetSystem(0);
    }
}

void LONG_CALL PermanentDeath_PostBattleCommitHook(
    struct BattleSetup *setup,
    FieldSystem *fieldSystem)
{
    if (setup->battleType & BATTLE_TYPE_DEBUG) {
        return;
    }

    BattleSetup_CommitToSave(setup, fieldSystem);

    // BugContest_Delete restores the persistent party only when the Contest
    // ends, so its pass is deliberately deferred to that command wrapper.
    if (setup->battleType & BATTLE_TYPE_BUG_CONTEST) {
        return;
    }

    if (setup->battleType & PERMANENT_DEATH_EXCLUDED_BATTLE_TYPES) {
        return;
    }

    PermanentDeath_ProcessPartyAfterBattle(fieldSystem->savedata);
}

#else

// The common script archive retains its permanent-death commands when the
// feature is disabled, so only their four small query/finalization stubs are
// required. Hook-only entry points have no disabled definitions.
BOOL PermanentDeath_HasPendingNotification(void)
{
    return FALSE;
}

u8 PermanentDeath_GetRecoveredPartySlot(void)
{
    return 0xFF;
}

BOOL PermanentDeath_ShouldEndRun(SaveData *saveData)
{
    (void)saveData;
    return FALSE;
}

void PermanentDeath_FinishNotification(BOOL endRun)
{
    (void)endRun;
}

#endif // IMPLEMENT_PERMANENT_DEATH
