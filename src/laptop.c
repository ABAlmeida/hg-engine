#include "../include/laptop.h"

#include "../include/config.h"
#include "../include/item.h"
#include "../include/pokemon.h"
#include "../include/task.h"

#define ITEM_USE_ERROR_NOT_NOW             3
#define ITEM_MENU_STATE_EXIT_TO_FIELD_TASK 12
#define MAP_LOAD_TYPE_OVERWORLD            0
#define LAPTOP_PC_SCRIPT_ID                 2076

_Static_assert(offsetof(FieldSystem, mapLoadType) == 0x70, "FieldSystem map-load type offset changed");
_Static_assert(sizeof(FieldSystem) == 0x128, "FieldSystem size changed");

extern void LONG_CALL StartScriptFromMenu(
    TaskManager *taskManager,
    u16 scriptId,
    void *lastInteracted);

#ifdef IMPLEMENT_LAPTOP

static BOOL Task_LaptopPC(TaskManager *taskManager)
{
    StartScriptFromMenu(taskManager, LAPTOP_PC_SCRIPT_ID, NULL);
    return FALSE;
}

u32 LONG_CALL ItemCheckUseFunc_Laptop(const struct ItemCheckUseData *checkData)
{
    // Only ordinary, disconnected overworld maps can safely yield to the
    // complete PC app chain. Special map-load modes and an active online
    // communications manager receive the standard field-item rejection.
    if (checkData->fieldSystem->mapLoadType != MAP_LOAD_TYPE_OVERWORLD
        || sub_02037D78()) {
        return ITEM_USE_ERROR_NOT_NOW;
    }

    return 0;
}

void LONG_CALL ItemMenuUseFunc_Laptop(
    struct ItemMenuUseData *data,
    const struct ItemCheckUseData *checkData UNUSED)
{
    FieldSystem *fieldSystem = data->taskManager->fieldSystem;
    struct BagViewAppWork *env = data->taskManager->env;

    FieldSystem_LoadFieldOverlay(fieldSystem);
    env->atexit_TaskFunc = Task_LaptopPC;
    env->atexit_TaskEnv = NULL;
    env->state = ITEM_MENU_STATE_EXIT_TO_FIELD_TASK;
}

BOOL LONG_CALL ItemFieldUseFunc_Laptop(struct ItemFieldUseData *data)
{
    // The item is deliberately not registrable, but keep this entry defensive
    // if another caller ever attempts to invoke its field callback directly.
    if (ItemCheckUseFunc_Laptop(&data->dat) != 0) {
        return FALSE;
    }

    FieldSystem_CreateTask(data->fieldSystem, Task_LaptopPC, NULL);
    return FALSE;
}

#else

u32 LONG_CALL ItemCheckUseFunc_Laptop(const struct ItemCheckUseData *checkData UNUSED)
{
    return ITEM_USE_ERROR_NOT_NOW;
}

void LONG_CALL ItemMenuUseFunc_Laptop(
    struct ItemMenuUseData *data UNUSED,
    const struct ItemCheckUseData *checkData UNUSED)
{
}

BOOL LONG_CALL ItemFieldUseFunc_Laptop(struct ItemFieldUseData *data UNUSED)
{
    return FALSE;
}

#endif
