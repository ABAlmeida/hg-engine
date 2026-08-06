#include "../include/bag.h"
#include "../include/battle.h"
#include "../include/config.h"
#include "../include/machine_field_actions.h"
#include "../include/overlay.h"
#include "../include/party_menu.h"
#include "../include/pokemon.h"
#include "../include/stat_training_items.h"
#include "../include/message.h"
#include "../include/types.h"
#include "../include/window.h"
#include "../include/constants/file.h"
#include "../include/constants/item.h"
#include "../include/constants/moves.h"
#include "../include/constants/species.h"

extern const u16 sButtonFrameTileOffsets[];
extern const u8 sButtonRects[][4];
extern const WindowTemplate sButtonWindowTemplates[];
extern const s8 sButtonWindowIDs[][2][8];

// mirrors the button layout
static const u8 sPartyMenuRotomCatalogFormOrder[] = {3, 4, 5, /*quit*/ 0, 0, 1, 2};

u32 LONG_CALL getButtonColorDepressed(int selection);
u32 LONG_CALL getButtonColorRaised(int selection);
void PartyMenu_ShowRotomCatalogList(struct PartyMenu *partyMenu);

#ifdef IMPLEMENT_LEVEL_CAP

#define LEVEL_TO_CAP_RESUME_SENTINEL (-1)

#define START_MENU_FIELD_MOVE_CHECK_DATA_OFFSET 0x370
#define START_MENU_EXIT_ENVIRONMENT_OFFSET      0x380
#define START_MENU_EXIT_ENVIRONMENT_2_OFFSET    0x384

typedef int (*PartyMenuStateFunc)(struct PartyMenu *);
typedef void (*StartMenuExitTaskFunc)(void *);

static BOOL sLevelToCapActive;

static int LevelToCap_StartNextLevel(struct PartyMenu *partyMenu);
static void LevelToCap_RestorePartySelectionUI(struct PartyMenu *partyMenu);

#endif

#ifdef IMPLEMENT_MACHINE_FIELD_ACTIONS

static BOOL PartyMenu_FieldMoveWasAdded(
    const u16 *addedMoves,
    u8 numAddedMoves,
    u16 moveId)
{
    u8 i;

    for (i = 0; i < numAddedMoves; i++) {
        if (addedMoves[i] == moveId) {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL PartyMenu_TryAddFieldMove(
    struct PartyMenu *partyMenu,
    u8 *items,
    u8 *numItems,
    u16 *addedMoves,
    u8 *numFieldMoves,
    u16 moveId)
{
    u8 fieldEffect;

    if (*numItems >= MAX_BUTTONS_IN_PARTY_MENU
        || *numFieldMoves >= MAX_MON_MOVES
        || PartyMenu_FieldMoveWasAdded(
            addedMoves,
            *numFieldMoves,
            moveId)) {
        return FALSE;
    }

    fieldEffect = MoveId_GetFieldEffectId(moveId);
    if (fieldEffect == 0xFF) {
        return FALSE;
    }

    items[(*numItems)++] = fieldEffect;
    PartyMenu_ContextMenuAddFieldMove(
        partyMenu,
        moveId,
        *numFieldMoves);
    addedMoves[(*numFieldMoves)++] = moveId;
    return TRUE;
}

#endif

u8 LONG_CALL sub_0207B0B0(struct PartyMenu *wk, u8 *buf)
{
    struct PartyPokemon *pp = Party_GetMonByIndex(wk->args->party, wk->partyMonIndex);
    u16 move;
    u8 fieldMoveIndex = 0;
    u8 i;
    u8 count = 0;
#ifdef IMPLEMENT_LEVEL_CAP
    BOOL showLevelToCap;
#endif
#ifdef IMPLEMENT_MACHINE_FIELD_ACTIONS
    u16 addedFieldMoves[MAX_MON_MOVES] = { MOVE_NONE };
    u32 machineActionIndex;
#endif

    u8 isEgg = GetMonData(pp, MON_DATA_IS_EGG, NULL);
    u32 item = GetMonData(pp, MON_DATA_HELD_ITEM, NULL);

    buf[count] = PARTY_MON_CONTEXT_MENU_SUMMARY;
    ++count;
    if (!FieldSystem_MapIsBattleTowerMultiPartnerSelectRoom(wk->args->fieldSystem))
    {
#ifdef IMPLEMENT_LEVEL_CAP
        showLevelToCap = Pokemon_CanLevelToCap(pp);
        if (showLevelToCap) {
            buf[count] = PARTY_MON_CONTEXT_MENU_LEVEL_TO_CAP;
            ++count;
        }
#endif
        buf[count] = PARTY_MON_CONTEXT_MENU_SWITCH;
        ++count;
        if (!isEgg)
        {
            if (IS_ITEM_MAIL(item))
            {
                buf[count] = PARTY_MON_CONTEXT_MENU_MAIL;
            }
            else
            {
                buf[count] = PARTY_MON_CONTEXT_MENU_ITEM;
            }
            ++count;

#ifndef IMPLEMENT_LEVEL_CAP
            buf[count] = PARTY_MON_CONTEXT_MENU_QUIT;
            ++count;
#endif

#ifdef IMPLEMENT_MACHINE_FIELD_ACTIONS
            // Show only owned machine actions that the engine says are usable
            // in the current field context. This keeps the bounded context menu
            // focused instead of appending every HM to every Pokemon.
            for (machineActionIndex = 0;
                 machineActionIndex < MachineFieldAction_GetCount();
                 machineActionIndex++) {
                move = MachineFieldAction_GetMove(machineActionIndex);
                if (MachineFieldAction_IsUsable(
                        wk->args->fieldSystem->savedata,
                        move,
                        wk->args->fieldMoveCheckData)) {
                    PartyMenu_TryAddFieldMove(
                        wk,
                        buf,
                        &count,
                        addedFieldMoves,
                        &fieldMoveIndex,
                        move);
                }
            }
#endif

            for (i = 0; i < MAX_MON_MOVES; ++i)
            {
                move = GetMonData(pp, MON_DATA_MOVE1 + i, NULL);
                if (move == MOVE_NONE)
                {
                    break;
                }

#ifdef IMPLEMENT_MACHINE_FIELD_ACTIONS
                PartyMenu_TryAddFieldMove(
                    wk,
                    buf,
                    &count,
                    addedFieldMoves,
                    &fieldMoveIndex,
                    move);
#else
                u8 fieldEffect;

                fieldEffect = MoveId_GetFieldEffectId(move);
                if (fieldEffect != 0xFF)
                {
                    buf[count] = fieldEffect;
                    ++count;
                    PartyMenu_ContextMenuAddFieldMove(wk, move, fieldMoveIndex);
                    ++fieldMoveIndex;
                }
#endif
            }
        }
        else
        {
            buf[count] = PARTY_MON_CONTEXT_MENU_QUIT;
            ++count;
        }
    }
    else
	{
        buf[count] = PARTY_MON_CONTEXT_MENU_QUIT;
        ++count;
    }

    return count;
}

void LONG_CALL sub_0207AFC4(struct PartyMenu *wk)
{
    ClearFrameAndWindow2(&wk->windows[PARTY_MENU_WINDOW_ID_32], TRUE);

    u8 *buf;
    buf = sys_AllocMemory(HEAP_ID_PARTY_MENU, MAX_BUTTONS_IN_PARTY_MENU);
    u8 numItems;

    switch (wk->args->context)//(partyMenu->args->context)
    {
    case PARTY_MENU_CONTEXT_0:
        numItems = sub_0207B0B0(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_UNION_ROOM_BATTLE_SELECT:
    case PARTY_MENU_CONTEXT_17:
        numItems = sub_0207B23C(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_ATTACH_CAPSULE:
        numItems = PartyMenu_SetContextMenuItems_GiveCapsule(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_18:
        numItems = sub_0207B1C8(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_SPIN_TRADE:
        numItems = PartyMenu_SetContextMenuItems_SpinTrade(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_BATTLE_HALL:
        numItems = PartyMenu_SetContextMenuItems_BattleHallEntry(wk, buf);
        break;
    case PARTY_MENU_CONTEXT_23:
        numItems = sub_0207B2DC(wk, buf);
        break;
    default:
        numItems = sub_0207B200(wk, buf);
        break;
    }

    PartyMenu_OpenContextMenu(wk, buf, numItems);
#ifdef IMPLEMENT_LEVEL_CAP
    for (u8 i = 0; i < numItems; ++i) {
        if (buf[i] == PARTY_MON_CONTEXT_MENU_LEVEL_TO_CAP) {
            wk->listMenuItems[i].value = (u32)PartyMonContextMenuAction_LevelToCap;
            break;
        }
    }
#endif
    Heap_FreeExplicit(HEAP_ID_PARTY_MENU, buf);
    sub_0207D1C8(wk);
    PartyMenu_PrintMessageOnWindow33(wk, -1, TRUE);
    thunk_Sprite_SetPalIndex(wk->sprites[PARTY_MENU_SPRITE_ID_CURSOR], 1);
}

/*
 * @brief hooks rare candy usage in the bag to allow for repeated use without returning to the bag between each
 * thanks to yako for the for the format
 */
int PartyMenu_ItemUseFunc_LevelUpLearnMovesLoop_Case6(struct PartyMenu *wk) {
    struct PartyPokemon *mon = Party_GetMonByIndex(wk->args->party, wk->partyMonIndex);
    wk->args->species = GetMonEvolution(wk->args->party, mon, EVOCTX_LEVELUP, EVO_NONE, (int *)&wk->args->evoMethod);
    if (wk->args->species != SPECIES_NONE) {
        wk->args->selectedAction = 0x9;
        return 0x20;
    }
#ifdef IMPLEMENT_LEVEL_CAP
    if (sLevelToCapActive) {
        if (Pokemon_CanLevelToCap(mon)) {
            return LevelToCap_StartNextLevel(wk);
        }

        sLevelToCapActive = FALSE;
        wk->args->context = PARTY_MENU_CONTEXT_0;
        wk->args->selectedAction = PARTY_MENU_ACTION_RETURN_0;
        ClearFrameAndWindow2(&wk->windows[PARTY_MENU_WINDOW_ID_34], TRUE);
        PartyMenu_PrintMessageOnWindow32(wk, 29, TRUE);
        LevelToCap_RestorePartySelectionUI(wk);
        return PARTY_MENU_STATE_1;
    }
#endif
    wk->args->selectedAction = 0x0;
    if (Bag_HasItem(wk->args->bag, wk->args->itemId, 1, HEAP_ID_PARTY_MENU)) {
        ClearFrameAndWindow2(&wk->windows[34], TRUE);
        PartyMenu_PrintMessageOnWindow32(wk, 33, TRUE); // message index in 300.txt
        return 0x4;
    }
    return 0x20;
}

#ifdef IMPLEMENT_LEVEL_CAP

static void LevelToCap_RestorePartySelectionUI(struct PartyMenu *partyMenu)
{
    partyMenu->topScreenPanelShow = FALSE;
    thunk_Sprite_SetPalIndex(partyMenu->sprites[PARTY_MENU_SPRITE_ID_CURSOR], 0);
}

/**
 *  @brief begin one native level-up presentation for the Level to Cap command
 *
 *  @param partyMenu active party menu
 *  @return next PartyMenuState
 */
static int LevelToCap_StartNextLevel(struct PartyMenu *partyMenu)
{
    struct PartyPokemon *mon = Party_GetMonByIndex(partyMenu->args->party, partyMenu->partyMonIndex);
    PartyMenuStateFunc levelUpFunc = (PartyMenuStateFunc)(0x02081A74 | 1);

    if (!Pokemon_CanLevelToCap(mon)) {
        sLevelToCapActive = FALSE;
        partyMenu->args->context = PARTY_MENU_CONTEXT_0;
        PartyMenu_PrintMessageOnWindow32(partyMenu, 29, TRUE);
        LevelToCap_RestorePartySelectionUI(partyMenu);
        return PARTY_MENU_STATE_1;
    }

    partyMenu->args->levelUpMoveSearchState = 0;
    partyMenu->args->itemId = ITEM_NONE;
    partyMenu->args->context = PARTY_MENU_CONTEXT_0;
    return levelUpFunc(partyMenu);
}

/**
 *  @brief handle the contextual Level to Cap party-menu action
 *
 *  @param partyMenu active party menu
 *  @param pState destination for the next PartyMenuState
 */
void LONG_CALL PartyMonContextMenuAction_LevelToCap(struct PartyMenu *partyMenu, int *pState)
{
    ClearFrameAndWindow2(&partyMenu->windows[PARTY_MENU_WINDOW_ID_33], TRUE);
    PartyMenu_DeleteContextMenuAndList(partyMenu);
    LevelToCap_RestorePartySelectionUI(partyMenu);
    sLevelToCapActive = TRUE;
    *pState = LevelToCap_StartNextLevel(partyMenu);
}

/**
 *  @brief replace Rare Candy mutation with one cap-aware level while the command is active
 */
BOOL LONG_CALL LevelToCap_UseItemOnMonInParty(struct Party *party,
    u16 itemId,
    s32 partySlot,
    u8 moveSlot,
    u16 mapSection,
    u32 heapId)
{
    typedef BOOL (*UseItemOnMonInPartyFunc)(struct Party *, u16, s32, u8, u16, u32);
    UseItemOnMonInPartyFunc useItem = (UseItemOnMonInPartyFunc)(0x020908AC | 1);

    if (sLevelToCapActive && itemId == ITEM_NONE) {
        return Pokemon_LevelToCapOneLevel(Party_GetMonByIndex(party, partySlot));
    }

    if (StatTrainingItem_IsHandled(itemId)) {
        return StatTrainingItem_UseOnMon(
            Party_GetMonByIndex(party, partySlot), itemId, mapSection);
    }

    return useItem(party, itemId, partySlot, moveSlot, mapSection, heapId);
}

/**
 *  @brief resume a Level to Cap run after an evolution has reopened the party menu
 *
 *  @return next PartyMenuState, or -1 when vanilla move-learning resume should run
 */
int LONG_CALL LevelToCap_TryResumePartyMenu(struct PartyMenu *partyMenu)
{
    if (!sLevelToCapActive
        || partyMenu->args->levelUpMoveSearchState != LEVEL_TO_CAP_RESUME_SENTINEL) {
        return -1;
    }

    partyMenu->args->levelUpMoveSearchState = 0;
    return LevelToCap_StartNextLevel(partyMenu);
}

BOOL LONG_CALL LevelToCap_IsActive(void)
{
    return sLevelToCapActive;
}

/**
 *  @brief reopen the selected Pokemon after its evolution instead of returning to the Bag
 */
void LONG_CALL LevelToCap_AfterEvolution(void *startMenu, FieldSystem *fieldSystem)
{
    typedef PartyMenuArgs *(*PartyMenuLaunchFunc)(FieldSystem *, void *, u8);
    typedef void (*SetExitTaskFunc)(void *, StartMenuExitTaskFunc);

    u8 *startMenuBytes = startMenu;
    void **environment = (void **)(startMenuBytes + START_MENU_EXIT_ENVIRONMENT_OFFSET);
    void **environment2 = (void **)(startMenuBytes + START_MENU_EXIT_ENVIRONMENT_2_OFFSET);
    int partySlot = *(int *)(*environment2);
    PartyMenuLaunchFunc launchPartyMenu = (PartyMenuLaunchFunc)(0x0203E550 | 1);
    SetExitTaskFunc setExitTask = (SetExitTaskFunc)(0x0203C8F0 | 1);
    PartyMenuArgs *args;

    args = launchPartyMenu(fieldSystem,
        startMenuBytes + START_MENU_FIELD_MOVE_CHECK_DATA_OFFSET,
        (u8)partySlot);
    args->context = PARTY_MENU_CONTEXT_REPLACE_MOVE_LEVELUP;
    args->levelUpMoveSearchState = LEVEL_TO_CAP_RESUME_SENTINEL;
    *environment = args;

    sys_FreeMemoryEz(*environment2);
    *environment2 = NULL;
    setExitTask(startMenu, (StartMenuExitTaskFunc)(0x0203CA9C | 1));
}

#else

// This shared party-item hook also owns stat-training items when level caps
// are disabled, so it must continue to delegate every other item to the ROM.
BOOL LONG_CALL LevelToCap_UseItemOnMonInParty(struct Party *party,
    u16 itemId,
    s32 partySlot,
    u8 moveSlot,
    u16 mapSection,
    u32 heapId)
{
    typedef BOOL (*UseItemOnMonInPartyFunc)(struct Party *, u16, s32, u8, u16, u32);
    UseItemOnMonInPartyFunc useItem = (UseItemOnMonInPartyFunc)(0x020908AC | 1);

    if (StatTrainingItem_IsHandled(itemId)) {
        return StatTrainingItem_UseOnMon(
            Party_GetMonByIndex(party, partySlot), itemId, mapSection);
    }

    return useItem(party, itemId, partySlot, moveSlot, mapSection, heapId);
}

int LONG_CALL LevelToCap_TryResumePartyMenu(struct PartyMenu *partyMenu UNUSED)
{
    return -1;
}

BOOL LONG_CALL LevelToCap_IsActive(void)
{
    return FALSE;
}

void LONG_CALL LevelToCap_AfterEvolution(void *startMenu UNUSED, FieldSystem *fieldSystem UNUSED)
{
}

#endif // IMPLEMENT_LEVEL_CAP

/*
 * @brief hooks into the ending of pokeheartgold PartyMenu_ItemUseFunc_WaitTextPrinterThenExit
 * to allow for item reuse if not an evo item and the bag has more of the item
 */
int PartyMenu_ItemUseFunc_ReuseItem(struct PartyMenu *wk) {
    wk->args->selectedAction = 0;
    if (GetItemData(wk->args->itemId, ITEM_PARAM_EVOLUTION, HEAP_ID_PARTY_MENU) == 0 && Bag_HasItem(wk->args->bag, wk->args->itemId, 1, HEAP_ID_PARTY_MENU)) {
        ClearFrameAndWindow2(&wk->windows[34], TRUE);
        PartyMenu_PrintMessageOnWindow32(wk, 33, TRUE); // message index in 300.txt
        return 0x4;
    }
    return 0x20;
}

void PartyMenu_LearnMoveToSlot(struct PartyMenu *partyMenu, struct PartyPokemon *mon, int moveIdx) {
    int data = partyMenu->args->moveId;
    SetMonData(mon, MON_DATA_MOVE1 + moveIdx, &data);
    data = 0;
    SetMonData(mon, MON_DATA_MOVE1PPUP + moveIdx, &data);
    data = GetMoveMaxPP(partyMenu->args->moveId, 0);
    SetMonData(mon, MON_DATA_MOVE1PP + moveIdx, &data);
    if (partyMenu->args->itemId != ITEM_NONE) {
#ifdef REUSABLE_TMS
    BOOL consumeItem = IS_ITEM_TR(partyMenu->args->itemId);
#else
    BOOL consumeItem = IS_ITEM_TM(partyMenu->args->itemId) || IS_ITEM_TR(partyMenu->args->itemId);
#endif // REUSABLE_TMS
        if (consumeItem) {
            Bag_TakeItem(partyMenu->args->bag, partyMenu->args->itemId, 1, HEAP_ID_PARTY_MENU);
        }
        MonApplyFriendshipMod(mon, 4, PartyMenu_GetCurrentMapSec(partyMenu));
        ApplyMonMoodModifier(mon, 3);
    }
}

void LONG_CALL PartyMonContextMenuAction_RotomCatalog(struct PartyMenu *partyMenu, int *pState)
{
    u8 form = 0;
    u8 sel = partyMenu->contextMenuButtonAnim.selection;
    if (sel <= 6) {
        form = sPartyMenuRotomCatalogFormOrder[sel];
    }
    partyMenu->args->species = PokeOtherFormMonsNoGet(SPECIES_ROTOM, form);
    Mon_UpdateRotomForm(Party_GetMonByIndex(partyMenu->args->party, partyMenu->partyMonIndex), form, 0);
    // TODO eventually expand PartyMenu_FormChangeScene_Begin with animations for Rotom
    PartyMenu_FormChangeScene_Begin(partyMenu);
    PartyMenu_DeleteContextMenuAndList(partyMenu);
    *pState = PARTY_MENU_STATE_FORM_CHANGE_ANIM;
}

void LONG_CALL PartyMonContextMenuAction_QuitToBag(struct PartyMenu *partyMenu, int *pState)
{
    // clean up submenu UI elements
    ClearFrameAndWindow2(&partyMenu->windows[PARTY_MENU_WINDOW_ID_33], TRUE);
    PartyMenu_DeleteContextMenuAndList(partyMenu);
    PartyMenu_DisableMainScreenBlend_AfterYesNo();

    // fully quit out rather than just quitting the submenu
    partyMenu->args->selectedAction = PARTY_MENU_ACTION_RETURN_0;
    *pState = PARTY_MENU_STATE_BEGIN_EXIT;
}

int LONG_CALL PartyMenu_HandleUseItemOnMon(struct PartyMenu *partyMenu)
{
    u32 ovyId, target, offset;
    u16 (*internalFunc)(struct PartyMenu *);

    ovyId = OVERLAY_PARTY_HANDLEUSEITEMONMON;
    offset = 0x023C0400 | 1;

    HandleLoadOverlay(ovyId, 2);
    internalFunc = (u16(*)(struct PartyMenu *))(offset);
    target = internalFunc(partyMenu);
    UnloadOverlayByID(ovyId);

    return target;
}

void LONG_CALL PartyMenu_PrintContextMenuItemText(struct PartyMenu *partyMenu, struct PartyMenuContextMenu *contextMenu, int numItems, int selection, int state, BOOL depressed)
{
    u32 color;
    u32 y;
    u32 x = 0;
    u32 fillValue;
    u8 windowId;

    windowId = sButtonWindowIDs[numItems - 1][state][selection];

    if (windowId == 7) {
        if (depressed == FALSE) {
            fillValue = 4;
            color = MAKE_TEXT_COLOR(14, 15, 4);
        } else {
            fillValue = 11;
            color = MAKE_TEXT_COLOR(14, 15, 11);
        }
        y = 4;
        x = FontID_String_GetCenterAlignmentX(4, contextMenu->items[selection].text, 0, GetWindowWidth(&partyMenu->contextMenuButtonWindows[windowId]) * 8);
    } else {
        if (partyMenu->args->itemId == ITEM_ROTOM_CATALOG) {
            // use the normal white colored text instead of blue field move text
            if (depressed == FALSE) {
                fillValue = 4;
                color = MAKE_TEXT_COLOR(14, 15, 4);
            } else {
                fillValue = 11;
                color = MAKE_TEXT_COLOR(14, 15, 11);
            }
        } else {
            if (depressed == FALSE) {
                fillValue = 4;
                color = getButtonColorRaised(selection);
            } else {
                fillValue = 11;
                color = getButtonColorDepressed(selection);
            }
        }
        y = 0;
    }
    FillWindowPixelBuffer(&partyMenu->contextMenuButtonWindows[windowId], fillValue);
    AddTextPrinterParameterizedWithColor(&partyMenu->contextMenuButtonWindows[windowId], 4, contextMenu->items[selection].text, x, y, TEXT_SPEED_NOTRANSFER, color, NULL);
    ScheduleWindowCopyToVram(&partyMenu->contextMenuButtonWindows[windowId]);
}

void LONG_CALL sub_0207E3A8(struct PartyMenu *partyMenu, int numItems, int selection, int state, int frameType)
{
    u16 tiles[8];
    s8 id = sButtonWindowIDs[numItems - 1][state][selection];

    // shallow copy vanilla rectangle
    u8 tempRect[4];
    const u8 *rect = sButtonRects[id];
    tempRect[0]=rect[0];
    tempRect[1]=rect[1];
    tempRect[2]=rect[2];
    tempRect[3]=rect[3];

    // nudge down one tile to line up columns
    if (partyMenu->args->itemId == ITEM_ROTOM_CATALOG) {
        if (id == 0 || id == 1 || id == 2) {
            tempRect[1] -= 1;
        }
    }

    u32 tileStart;
    if (frameType == 0) {
        tileStart = 0x2000;
    } else if (frameType == 1) {
        tileStart = 0x2009;
    } else {
        tileStart = 0x2012;
    }
    tileStart += 10;
    for (int i = 0; i < 8; ++i) { // MAX_BUTTONS_IN_PARTY_MENU ?
        tiles[i] = tileStart + sButtonFrameTileOffsets[i];
    }

    LoadRectToBgTilemapRect(partyMenu->bgConfig, 0, &tiles[0], tempRect[0], tempRect[1], 1, 1);
    LoadRectToBgTilemapRect(partyMenu->bgConfig, 0, &tiles[1], tempRect[0] + tempRect[2] - 1, tempRect[1], 1, 1);
    LoadRectToBgTilemapRect(partyMenu->bgConfig, 0, &tiles[2], tempRect[0], tempRect[1] + tempRect[3] - 1, 1, 1);
    LoadRectToBgTilemapRect(partyMenu->bgConfig, 0, &tiles[3], tempRect[0] + tempRect[2] - 1, tempRect[1] + tempRect[3] - 1, 1, 1);
    FillBgTilemapRect(partyMenu->bgConfig, 0, tiles[4], tempRect[0], tempRect[1] + 1, 1, tempRect[3] - 2, TILEMAP_FILL_OVWT_PAL);
    FillBgTilemapRect(partyMenu->bgConfig, 0, tiles[5], tempRect[0] + tempRect[2] - 1, tempRect[1] + 1, 1, tempRect[3] - 2, TILEMAP_FILL_OVWT_PAL);
    FillBgTilemapRect(partyMenu->bgConfig, 0, tiles[6], tempRect[0] + 1, tempRect[1], tempRect[2] - 2, 1, TILEMAP_FILL_OVWT_PAL);
    FillBgTilemapRect(partyMenu->bgConfig, 0, tiles[7], tempRect[0] + 1, tempRect[1] + tempRect[3] - 1, tempRect[2] - 2, 1, TILEMAP_FILL_OVWT_PAL);
}

void LONG_CALL PartyMenu_StartContextMenuButtonPressAnim_FromCursorObj(struct PartyMenu *partyMenu, PartyMenuContextMenuCursor *cursor, int followUpState)
{
    int followUpStateTmp = followUpState;

    // intercept cancel signals sent from the B button to properly route to the Quit button for specific items
    u8 idx = cursor->numItems - 1;
    if (partyMenu->args && partyMenu->args->itemId == ITEM_ROTOM_CATALOG && followUpState == LIST_CANCEL) {
        for (int i = 0; i < cursor->numItems; i++) {
            if ((u32)cursor->menu.items[i].value == (u32)PartyMonContextMenuAction_QuitToBag) {
                idx = i;
                break;
            }
        }
        cursor->selection = idx;
        followUpStateTmp = cursor->menu.items[idx].value;
    }

    PartyMenuContextButtonAnimData *animData = &partyMenu->contextMenuButtonAnim;

    animData->autoAnimTimer = 0;
    animData->buttonAnimState = 0;
    animData->template = &cursor->menu;
    animData->numItems = cursor->numItems;
    animData->selection = cursor->selection;
    animData->state = cursor->state;
    animData->followUpState = followUpStateTmp;
    animData->active = TRUE;
}

/**
 *  @brief check if a rotom catalog can be used on a PartyPokemon
 *
 *  @param pp PartyPokemon to check reveal glass against
 *  @return TRUE if rotom catalog can be used; FALSE otherwise
 */
BOOL CanUseRotomCatalog(struct PartyPokemon *pp)
{
    return GetMonData(pp, MON_DATA_SPECIES, NULL) == SPECIES_ROTOM;
}
