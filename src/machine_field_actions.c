#include "../include/machine_field_actions.h"

#include "../include/bag.h"
#include "../include/config.h"
#include "../include/pokemon.h"
#include "../include/save.h"
#include "../include/script.h"
#include "../include/constants/item.h"
#include "../include/constants/moves.h"

#ifdef IMPLEMENT_MACHINE_FIELD_ACTIONS

typedef u32 (*FieldMoveCheckFunc)(const void *fieldMoveCheckData);

typedef struct MachineFieldActionDefinition {
    u16 moveId;
    u16 itemId;
    u8 fieldMoveIndex;
} MachineFieldActionDefinition;

typedef enum FieldMoveFuncType {
    FIELD_MOVE_FUNC_USE = 0,
    FIELD_MOVE_FUNC_CHECK,
} FieldMoveFuncType;

typedef enum FieldMoveResponse {
    FIELD_MOVE_RESPONSE_OK = 0,
} FieldMoveResponse;

typedef enum MachineFieldMoveIndex {
    FIELD_MOVE_INDEX_CUT = 0,
    FIELD_MOVE_INDEX_FLY = 1,
    FIELD_MOVE_INDEX_SURF = 2,
    FIELD_MOVE_INDEX_STRENGTH = 3,
    FIELD_MOVE_INDEX_ROCK_SMASH = 4,
    FIELD_MOVE_INDEX_WATERFALL = 5,
    FIELD_MOVE_INDEX_ROCK_CLIMB = 6,
    FIELD_MOVE_INDEX_FLASH = 7,
    // Indices 8-11 are Teleport, Dig, Sweet Scent, and Chatter.
    FIELD_MOVE_INDEX_WHIRLPOOL = 12,
} MachineFieldMoveIndex;

enum {
    MACHINE_FIELD_ACTION_COUNT = 9,
};

void *LONG_CALL FieldMove_GetMoveFunc(
    FieldMoveFuncType funcType,
    u16 fieldMoveIndex);

static const MachineFieldActionDefinition sMachineFieldActions[] = {
    { MOVE_CUT, ITEM_HM01, FIELD_MOVE_INDEX_CUT },
    { MOVE_FLY, ITEM_HM02, FIELD_MOVE_INDEX_FLY },
    { MOVE_SURF, ITEM_HM03, FIELD_MOVE_INDEX_SURF },
    { MOVE_STRENGTH, ITEM_HM04, FIELD_MOVE_INDEX_STRENGTH },
    { MOVE_WHIRLPOOL, ITEM_HM05, FIELD_MOVE_INDEX_WHIRLPOOL },
    { MOVE_ROCK_SMASH, ITEM_HM06, FIELD_MOVE_INDEX_ROCK_SMASH },
    { MOVE_WATERFALL, ITEM_HM07, FIELD_MOVE_INDEX_WATERFALL },
    { MOVE_ROCK_CLIMB, ITEM_HM08, FIELD_MOVE_INDEX_ROCK_CLIMB },
    { MOVE_FLASH, ITEM_TM070, FIELD_MOVE_INDEX_FLASH },
};

_Static_assert(
    NELEMS(sMachineFieldActions) == MACHINE_FIELD_ACTION_COUNT,
    "Machine field-action table is out of sync");

static const MachineFieldActionDefinition *MachineFieldAction_FindDefinition(u16 moveId)
{
    u32 i;

    for (i = 0; i < NELEMS(sMachineFieldActions); i++) {
        if (sMachineFieldActions[i].moveId == moveId) {
            return &sMachineFieldActions[i];
        }
    }

    return NULL;
}

static BOOL MachineFieldAction_IsMachineOwned(
    const SaveData *saveData,
    u16 itemId)
{
    const BAG_DATA *bag = Sav2_Bag_get((void *)saveData);
    u32 i;

    // Every supported unlock is an HM or TM, so reading this pocket directly
    // avoids item-data allocation when the Party menu checks all nine actions.
    for (i = 0; i < NELEMS(bag->TMsHMs); i++) {
        if (bag->TMsHMs[i].id == itemId && bag->TMsHMs[i].quantity != 0) {
            return TRUE;
        }
    }

    return FALSE;
}

u32 MachineFieldAction_GetCount(void)
{
    return NELEMS(sMachineFieldActions);
}

u16 MachineFieldAction_GetMove(u32 index)
{
    if (index >= NELEMS(sMachineFieldActions)) {
        return MOVE_NONE;
    }

    return sMachineFieldActions[index].moveId;
}

BOOL MachineFieldAction_IsUsable(
    const SaveData *saveData,
    u16 moveId,
    const void *fieldMoveCheckData)
{
    const MachineFieldActionDefinition *definition =
        MachineFieldAction_FindDefinition(moveId);
    FieldMoveCheckFunc check;

    if (definition == NULL || fieldMoveCheckData == NULL) {
        return FALSE;
    }

    if (!MachineFieldAction_IsMachineOwned(saveData, definition->itemId)) {
        return FALSE;
    }

    check = (FieldMoveCheckFunc)FieldMove_GetMoveFunc(
        FIELD_MOVE_FUNC_CHECK,
        definition->fieldMoveIndex);
    return check != NULL && check(fieldMoveCheckData) == FIELD_MOVE_RESPONSE_OK;
}

static BOOL MachineFieldAction_FindUser(
    const SaveData *saveData,
    u16 moveId,
    u16 *partySlot)
{
    const MachineFieldActionDefinition *definition =
        MachineFieldAction_FindDefinition(moveId);
    struct Party *party;
    int firstNonEggSlot = MACHINE_FIELD_ACTION_PARTY_NOT_FOUND;
    int partyCount;
    int i;

    if (partySlot == NULL) {
        return FALSE;
    }

    *partySlot = MACHINE_FIELD_ACTION_PARTY_NOT_FOUND;
    party = SaveData_GetPlayerPartyPtr((void *)saveData);
    partyCount = PokeParty_GetPokeCount(party);

    // Remember the fallback while preserving a genuine move user's priority.
    for (i = 0; i < partyCount; i++) {
        struct PartyPokemon *mon = Party_GetMonByIndex(party, i);

        if (GetMonData(mon, MON_DATA_IS_EGG, NULL)) {
            continue;
        }

        if (firstNonEggSlot == MACHINE_FIELD_ACTION_PARTY_NOT_FOUND) {
            firstNonEggSlot = i;
        }

        if (MonHasMove(mon, moveId)) {
            *partySlot = i;
            return TRUE;
        }
    }

    if (definition == NULL
        || !MachineFieldAction_IsMachineOwned(saveData, definition->itemId)
        || firstNonEggSlot == MACHINE_FIELD_ACTION_PARTY_NOT_FOUND) {
        return FALSE;
    }

    *partySlot = firstNonEggSlot;
    return TRUE;
}

/**
 * Script command 141 asks for the first party slot containing a move. Field
 * scripts use the returned slot for the standard Pokemon name and animation,
 * so an owned machine falls back to a real non-Egg party member rather than
 * returning an invalid sentinel to those scripts.
 */
BOOL LONG_CALL MachineFieldAction_ScrCmdGetPartySlotWithMove(SCRIPTCONTEXT *ctx)
{
    u16 *partySlot = ScriptGetVarPointer(ctx);
    u16 moveId = ScriptGetVar(ctx);

    MachineFieldAction_FindUser(
        ctx->fsys->savedata,
        moveId,
        partySlot);
    return FALSE;
}

#endif // IMPLEMENT_MACHINE_FIELD_ACTIONS
