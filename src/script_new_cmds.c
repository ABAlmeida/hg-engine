#include "../include/config.h"
#include "../include/constants/file.h"
#include "../include/permanent_death.h"
#include "../include/repel.h"
#include "../include/roamer.h"
#include "../include/script.h"
#include "../include/types.h"

enum {
    SCRIPT_NEW_CMD_REPEL_USE = 0,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_PENDING = 1,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_RECOVERED_SLOT = 2,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_SHOULD_END = 3,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_FINISH = 4,
};

BOOL Script_RunNewCmd(SCRIPTCONTEXT *ctx)
{
    u8 sw = ScriptReadByte(ctx);
    u16 arg0 = ScriptReadHalfword(ctx);

    switch (sw) {
    case SCRIPT_NEW_CMD_REPEL_USE:;
#ifdef IMPLEMENT_REUSABLE_REPELS
        u16 most_recent_repel = Repel_GetMostRecent();
        SetScriptVar(arg0, most_recent_repel);
        Repel_Use(most_recent_repel, HEAPID_MAIN_HEAP);
#endif
        break;

    case SCRIPT_NEW_CMD_PERMANENT_DEATH_PENDING:
        SetScriptVar(arg0, PermanentDeath_HasPendingNotification());
        break;

    case SCRIPT_NEW_CMD_PERMANENT_DEATH_RECOVERED_SLOT:
        SetScriptVar(arg0, PermanentDeath_GetRecoveredPartySlot());
        break;

    case SCRIPT_NEW_CMD_PERMANENT_DEATH_SHOULD_END:
        SetScriptVar(
            arg0,
            PermanentDeath_ShouldEndRun(ctx->fsys->savedata));
        break;

    case SCRIPT_NEW_CMD_PERMANENT_DEATH_FINISH:
        PermanentDeath_FinishNotification(arg0 != 0);
        break;

    default:
        break;
    }

    return FALSE;
}

BOOL LONG_CALL ScrCmd_CreateRoamer(SCRIPTCONTEXT *ctx)
{
    u8 roamerNo = ScriptReadByte(ctx);
    Save_CreateRoamerByID(ctx->fsys->savedata, roamerNo);
    return FALSE;
}
