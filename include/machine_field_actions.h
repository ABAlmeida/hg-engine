#ifndef POKEHEARTLESSGOLD_MACHINE_FIELD_ACTIONS_H
#define POKEHEARTLESSGOLD_MACHINE_FIELD_ACTIONS_H

#include "types.h"

typedef struct SCRIPTCONTEXT SCRIPTCONTEXT;
typedef struct SaveData SaveData;

#define MACHINE_FIELD_ACTION_PARTY_NOT_FOUND 6

u32 MachineFieldAction_GetCount(void);
u16 MachineFieldAction_GetMove(u32 index);
BOOL MachineFieldAction_IsUsable(
    const SaveData *saveData,
    u16 moveId,
    const void *fieldMoveCheckData);
BOOL LONG_CALL MachineFieldAction_ScrCmdGetPartySlotWithMove(SCRIPTCONTEXT *ctx);

#endif // POKEHEARTLESSGOLD_MACHINE_FIELD_ACTIONS_H
