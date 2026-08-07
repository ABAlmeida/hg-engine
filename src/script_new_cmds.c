#include "../include/config.h"
#include "../include/constants/file.h"
#include "../include/permanent_death.h"
#include "../include/pokemon.h"
#include "../include/repel.h"
#include "../include/roamer.h"
#include "../include/save.h"
#include "../include/script.h"
#include "../include/types.h"

enum {
    SCRIPT_NEW_CMD_REPEL_USE = 0,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_PENDING = 1,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_RECOVERED_SLOT = 2,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_SHOULD_END = 3,
    SCRIPT_NEW_CMD_PERMANENT_DEATH_FINISH = 4,
    SCRIPT_NEW_CMD_CONFIGURE_GIFT_EGG = 5,
};

#define GIFT_EGG_IV_MASK 0x1F
#define GIFT_EGG_ABILITY_MASK 0x1FF

static void ConfigureGiftEgg(SCRIPTCONTEXT *ctx, u16 configVar)
{
    struct Party *party = SaveData_GetPlayerPartyPtr(ctx->fsys->savedata);
    int count = PokeParty_GetPokeCount(party);
    u16 config;
    u16 ability;
    u8 iv;
    u8 stat;

    if (count == 0) {
        return;
    }

    config = *GetVarPointer(ctx->fsys, configVar);
    iv = config & GIFT_EGG_IV_MASK;
    ability = (config >> 5) & GIFT_EGG_ABILITY_MASK;

    struct PartyPokemon *egg = Party_GetMonByIndex(party, count - 1);
    for (stat = 0; stat < 6; stat++) {
        SetMonData(egg, MON_DATA_HP_IV + stat, &iv);
    }

    // Preserve the ability through engine recalculations by recording whether
    // it belongs to the normal or hidden slot, not just its displayed value.
    u16 species = GetMonData(egg, MON_DATA_SPECIES, NULL);
    u16 form = GetMonData(egg, MON_DATA_FORM, NULL);
    u16 ability1 = PokeFormNoPersonalParaGet(species, form, PERSONAL_ABILITY_1);
    u16 ability2 = PokeFormNoPersonalParaGet(species, form, PERSONAL_ABILITY_2);
    u8 hiddenFlags = GetMonData(egg, MON_DATA_RESERVED_113, NULL);
    u16 abilityFlags = GetMonData(egg, MON_DATA_RESERVED_114, NULL);

    hiddenFlags &= ~DUMMY_P2_1_HIDDEN_ABILITY_MASK;
    abilityFlags &= ~DUMMY_P2_2_CHANGE_ABILITY_SLOT;

    if (ability == ability1 || (ability2 != 0 && ability == ability2)) {
        u16 selectedAbility = ability2 != 0
                && (GetMonData(egg, MON_DATA_PERSONALITY, NULL) & 1)
            ? ability2
            : ability1;
        if (selectedAbility != ability) {
            abilityFlags |= DUMMY_P2_2_CHANGE_ABILITY_SLOT;
        }
    } else if (ability == GetMonHiddenAbility(species, form)) {
        hiddenFlags |= DUMMY_P2_1_HIDDEN_ABILITY_MASK;
    }

    SetMonData(egg, MON_DATA_RESERVED_113, &hiddenFlags);
    SetMonData(egg, MON_DATA_RESERVED_114, &abilityFlags);
    SetMonData(egg, MON_DATA_ABILITY, &ability);
    RecalcPartyPokemonStats(egg);
}

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

    case SCRIPT_NEW_CMD_CONFIGURE_GIFT_EGG:
        ConfigureGiftEgg(ctx, arg0);
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
