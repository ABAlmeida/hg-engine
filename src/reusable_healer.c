#include "../include/reusable_healer.h"

#include "../include/config.h"
#include "../include/constants/file.h"
#include "../include/item.h"
#include "../include/message.h"
#include "../include/pokemon.h"
#include "../include/save.h"
#include "../include/task.h"
#include "../include/window.h"

#define ITEM_MENU_STATE_EXIT_TO_FIELD_TASK 12

typedef struct ReusableHealerMessageTaskData {
    struct Window window;
    String *string;
    u16 printerId;
    u16 state;
} ReusableHealerMessageTaskData;

_Static_assert(
    sizeof(ReusableHealerMessageTaskData) == 0x18,
    "Field item message task data layout is out of sync");

// This is the game's standard field-side Key Item message task. Keeping the
// matching private layout above lets the healer reuse the normal presentation.
BOOL LONG_CALL Task_PrintRegisteredKeyItemUseMessage(TaskManager *taskManager);

#ifdef IMPLEMENT_REUSABLE_HEALER

void LONG_CALL ReusableHealer_HealParty(struct Party *party)
{
    u32 status = 0;
    int i;

    for (i = 0; i < party->count; i++) {
        struct PartyPokemon *mon = Party_GetMonByIndex(party, i);
        u16 maxHp;

        if (GetMonData(mon, MON_DATA_IS_EGG, NULL)) {
            continue;
        }

        maxHp = (u16)GetMonData(mon, MON_DATA_MAXHP, NULL);
        SetMonData(mon, MON_DATA_HP, &maxHp);
        SetMonData(mon, MON_DATA_STATUS, &status);
        RestoreBoxMonPP(&mon->box);
    }
}

static BOOL Task_ReusableHealer(TaskManager *taskManager)
{
    FieldSystem *fieldSystem = taskManager->fieldSystem;
    ReusableHealerMessageTaskData *messageTask;
    MsgData *messageData;

    if (taskManager->state != 0) {
        return TRUE;
    }

    ReusableHealer_HealParty(SaveData_GetPlayerPartyPtr(fieldSystem->savedata));

    messageTask = sys_AllocMemory(HEAPID_WORLD, sizeof(ReusableHealerMessageTaskData));
    memset(messageTask, 0, sizeof(ReusableHealerMessageTaskData));
    messageData = NewMsgDataFromNarc(MSGDATA_LOAD_LAZY, ARC_MSG_DATA, MSG_DATA_BAG, HEAPID_WORLD);
    messageTask->string = NewString_ReadMsgData(messageData, MSG_DATA_BAG_REUSABLE_HEALER);
    DestroyMsgData(messageData);

    taskManager->state++;
    TaskManager_Call(taskManager, Task_PrintRegisteredKeyItemUseMessage, messageTask);
    return FALSE;
}

void LONG_CALL ItemMenuUseFunc_ReusableHealer(
    struct ItemMenuUseData *data,
    const struct ItemCheckUseData *checkData UNUSED)
{
    FieldSystem *fieldSystem = data->taskManager->fieldSystem;
    struct BagViewAppWork *env = data->taskManager->env;

    FieldSystem_LoadFieldOverlay(fieldSystem);
    env->atexit_TaskFunc = Task_ReusableHealer;
    env->atexit_TaskEnv = NULL;
    env->state = ITEM_MENU_STATE_EXIT_TO_FIELD_TASK;
}

#else

void LONG_CALL ReusableHealer_HealParty(struct Party *party UNUSED)
{
}

void LONG_CALL ItemMenuUseFunc_ReusableHealer(
    struct ItemMenuUseData *data UNUSED,
    const struct ItemCheckUseData *checkData UNUSED)
{
}

#endif
