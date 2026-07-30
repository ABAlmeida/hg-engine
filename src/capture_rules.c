#include "../include/capture_rules.h"

#include "../include/battle.h"
#include "../include/config.h"
#include "../include/permanent_death.h"
#include "../include/pokedex.h"
#include "../include/pokemon.h"
#include "../include/save.h"
#include "../include/script.h"
#include "../include/constants/species.h"

#ifdef IMPLEMENT_CAPTURE_RULES

#define POKEDEX_FLAG_BITS_PER_WORD 32

enum {
    BUG_CONTEST_CAUGHT_POKEMON = 1 << 0,
};

// Partial view of the original BugContest work structure. Only fields required
// to decide whether its final caught Pokemon is transferred are represented.
typedef struct BugContestWorkView {
    u32 heapId;
    struct SaveData *saveData;
    struct Party *partyBackup;
    struct Party *currentParty;
    struct PartyPokemon *caughtPokemon;
    u8 leadPokemonIndex;
    u8 originalPartyCount;
    u8 dayOfWeek;
    u8 state;
} BugContestWorkView;

_Static_assert(CAPTURE_AREA_WORD_COUNT == 8, "Capture area bitset must cover map sections 0 through 234");
_Static_assert(sizeof(CaptureRulesSave) == 36, "CaptureRulesSave layout changed");
_Static_assert(offsetof(struct BATTLE_PARAM, poke_party) == 0x4, "BATTLE_PARAM party offset changed");
_Static_assert(offsetof(struct BATTLE_PARAM, win_lose_flag) == 0x14, "BATTLE_PARAM outcome offset changed");
_Static_assert(offsetof(struct BATTLE_PARAM, trainer_id) == 0x18, "BATTLE_PARAM trainer-ID offset changed");
_Static_assert(offsetof(struct BATTLE_PARAM, trainer_data) == 0x28, "BATTLE_PARAM trainer-data offset changed");
_Static_assert(offsetof(struct BATTLE_PARAM, map_section) == 0x154, "BATTLE_PARAM map-section offset changed");
_Static_assert(offsetof(struct BATTLE_PARAM, savedata) == 0x1C0, "BATTLE_PARAM save-data offset changed");
_Static_assert(sizeof(struct BATTLE_PARAM) == 0x1C4, "BATTLE_PARAM size changed");
_Static_assert(offsetof(FieldSystem, savedata) == 0xC, "FieldSystem save-data offset changed");
_Static_assert(offsetof(FieldSystem, bugContest) == 0x118, "FieldSystem Bug Contest offset changed");
_Static_assert(sizeof(FieldSystem) == 0x128, "FieldSystem size changed");
_Static_assert(offsetof(BugContestWorkView, caughtPokemon) == 0x10, "Bug Contest Pokemon offset changed");
_Static_assert(offsetof(BugContestWorkView, state) == 0x17, "Bug Contest state offset changed");

// Transient permission for the active battle; reset before generation and when
// the battle controller reaches its terminal state.
static CapturePermission sEncounterPermission;

// Original-ROM functions resolved to their runtime addresses by rom.ld.
void LONG_CALL GenerateWildNonShinyAndAddToParty(
    u16 species,
    u8 level,
    int battler,
    BOOL forceOnePerfectIV,
    void *encounterGen,
    struct PartyPokemon *leadMon,
    struct BATTLE_PARAM *battleParam);
void *LONG_CALL BugContest_New(FieldSystem *fieldSystem, u32 weekday);
void LONG_CALL BugContest_Delete(void *bugContest);
void LONG_CALL sub_02093070(FieldSystem *fieldSystem);

static BOOL CaptureRules_IsValidArea(u32 mapSection)
{
    return mapSection < MAPSEC_COUNT;
}

static BOOL CaptureRules_CheckPokedexFlag(const u32 *flags, u16 species)
{
    u32 flagIndex = species - 1;

    return (flags[flagIndex / POKEDEX_FLAG_BITS_PER_WORD] & (1U << (flagIndex % POKEDEX_FLAG_BITS_PER_WORD))) != 0;
}

void CaptureRules_Init(CaptureRulesSave *captureRules)
{
    memset(captureRules, 0, sizeof(*captureRules));
}

static CaptureRulesSave *CaptureRules_GetSave(struct SaveData *saveData)
{
    return &Sav2_Misc_get(saveData)->captureRules;
}

static BOOL CaptureRules_IsAreaConsumed(const CaptureRulesSave *captureRules, u32 mapSection)
{
    if (!CaptureRules_IsValidArea(mapSection)) {
        return FALSE;
    }

    return (captureRules->consumedAreas[mapSection / CAPTURE_AREA_BITS_PER_WORD] & (1U << (mapSection % CAPTURE_AREA_BITS_PER_WORD))) != 0;
}

static BOOL CaptureRules_ConsumeArea(CaptureRulesSave *captureRules, u32 mapSection)
{
    u32 mask;
    u32 *word;

    if (!CaptureRules_IsValidArea(mapSection)) {
        return FALSE;
    }

    word = &captureRules->consumedAreas[mapSection / CAPTURE_AREA_BITS_PER_WORD];
    mask = 1U << (mapSection % CAPTURE_AREA_BITS_PER_WORD);
    if ((*word & mask) != 0) {
        return FALSE;
    }

    *word |= mask;
    return TRUE;
}

static BOOL CaptureRules_IsSpeciesDuplicate(struct SaveData *saveData, u16 species)
{
    const struct Save_DexData *pokedex;

    // The vanilla caught bitset represents only the 493 National Dex species.
    // Later IDs include Egg/placeholders and Generation 5+ values that would
    // index beyond the 16-word array.
    if (species == SPECIES_NONE || species > SPECIES_ARCEUS) {
        return FALSE;
    }

    pokedex = SaveData_GetDexPtr(saveData);
    return CaptureRules_CheckPokedexFlag(pokedex->get_flag, species);
}

void LONG_CALL CaptureRules_ResetEncounterState(void)
{
    sEncounterPermission = CAPTURE_PERMISSION_UNRESTRICTED;
}

CapturePermission LONG_CALL CaptureRules_GetEncounterPermission(void)
{
    return sEncounterPermission;
}

static void CaptureRules_EvaluateOrdinaryEncounter(struct BATTLE_PARAM *battleParam, struct PartyPokemon *wildMon)
{
    CaptureRulesSave *captureRules;
    u16 species;

    if (battleParam == NULL || battleParam->savedata == NULL || wildMon == NULL) {
        return;
    }

    if (MonIsShiny(wildMon)) {
        sEncounterPermission = CAPTURE_PERMISSION_ALLOWED_SHINY;
        return;
    }

    captureRules = CaptureRules_GetSave(battleParam->savedata);
    if (!CaptureRules_IsValidArea(battleParam->map_section)
        || CaptureRules_IsAreaConsumed(captureRules, battleParam->map_section)) {
        sEncounterPermission = CAPTURE_PERMISSION_BLOCKED_AREA;
        return;
    }

    species = GetMonData(wildMon, MON_DATA_SPECIES, NULL);
    if (CaptureRules_IsSpeciesDuplicate(battleParam->savedata, species)) {
        sEncounterPermission = CAPTURE_PERMISSION_BLOCKED_DUPLICATE;
        return;
    }

    // An eligible ordinary encounter consumes the area only after passing the
    // duplicate check, even if the player later defeats or flees from it.
    CaptureRules_ConsumeArea(captureRules, battleParam->map_section);
    sEncounterPermission = CAPTURE_PERMISSION_ALLOWED_STANDARD;
}

static void CaptureRules_EvaluateSafariEncounter(
    struct SaveData *saveData,
    u32 mapSection,
    struct PartyPokemon *wildMon)
{
    CaptureRulesSave *captureRules;
    u16 species;

    if (saveData == NULL || wildMon == NULL) {
        return;
    }

    if (MonIsShiny(wildMon)) {
        sEncounterPermission = CAPTURE_PERMISSION_ALLOWED_SHINY;
        return;
    }

    captureRules = CaptureRules_GetSave(saveData);
    if (!CaptureRules_IsValidArea(mapSection)
        || CaptureRules_IsAreaConsumed(captureRules, mapSection)) {
        sEncounterPermission = CAPTURE_PERMISSION_BLOCKED_AREA;
        return;
    }

    // Safari consumes its one opportunity before the duplicate check: the
    // first non-shiny Pokemon encountered is the only standard Safari chance.
    CaptureRules_ConsumeArea(captureRules, mapSection);

    species = GetMonData(wildMon, MON_DATA_SPECIES, NULL);
    if (CaptureRules_IsSpeciesDuplicate(saveData, species)) {
        sEncounterPermission = CAPTURE_PERMISSION_BLOCKED_DUPLICATE;
        return;
    }

    sEncounterPermission = CAPTURE_PERMISSION_ALLOWED_STANDARD;
}

void LONG_CALL CaptureRules_GenerateOrdinaryEncounter(
    u16 species,
    u8 level,
    int battler,
    BOOL forceOnePerfectIV,
    void *encounterGen,
    struct PartyPokemon *leadMon,
    struct BATTLE_PARAM *battleParam)
{
    struct Party *wildParty = NULL;
    int previousPartyCount = -1;

    CaptureRules_ResetEncounterState();

    if (battleParam != NULL && battler >= 0 && battler < BATTLER_MAX) {
        wildParty = battleParam->poke_party[battler];
        if (wildParty != NULL) {
            previousPartyCount = wildParty->count;
        }
    }

    GenerateWildNonShinyAndAddToParty(
        species,
        level,
        battler,
        forceOnePerfectIV,
        encounterGen,
        leadMon,
        battleParam);

    // Some generation attempts can fail before appending a Pokemon. Do not
    // evaluate an old party member in that case.
    if (wildParty == NULL || wildParty->count <= previousPartyCount) {
        return;
    }

    CaptureRules_EvaluateOrdinaryEncounter(
        battleParam,
        Party_GetMonByIndex(wildParty, wildParty->count - 1));
}

void LONG_CALL CaptureRules_InitializeSpecialBattle(struct BattleSystem *battleSystem)
{
    struct PartyPokemon *wildMon;
    struct SaveData *saveData;

    // Ordinary encounters were evaluated by the generation hook. Safari uses
    // a different generator, so initialize it once the battle party is loaded.
    if (sEncounterPermission != CAPTURE_PERMISSION_UNRESTRICTED
        || (BattleTypeGet(battleSystem) & BATTLE_TYPE_SAFARI) == 0
        || BattleWorkPokeCountGet(battleSystem, BATTLER_ENEMY) == 0) {
        return;
    }

    saveData = SaveBlock2_get();
    wildMon = Battle_GetClientPartyMon(battleSystem, BATTLER_ENEMY, 0);
    CaptureRules_EvaluateSafariEncounter(
        saveData,
        BattleWorkPlaceIDGet(battleSystem),
        wildMon);
}

BOOL LONG_CALL CaptureRules_ScrCmdBugContestAction(SCRIPTCONTEXT *ctx)
{
    u8 action = ScriptReadByte(ctx);
    u32 weekday = ScriptGetVar(ctx);
    FieldSystem *fieldSystem = ctx->fsys;
    CaptureRulesSave *captureRules =
        CaptureRules_GetSave(fieldSystem->savedata);

    if (action == 0) {
        fieldSystem->bugContest = BugContest_New(fieldSystem, weekday);
    } else {
        BugContestWorkView *bugContest = fieldSystem->bugContest;

        // Contest entry alone does not consume the allowance. Record it only
        // when the normal deletion path is about to retain a caught Pokemon.
        if (bugContest != NULL && (bugContest->state & BUG_CONTEST_CAUGHT_POKEMON) != 0) {
            if (captureRules->bugContestPokemonKept
                && !MonIsShiny(bugContest->caughtPokemon)) {
                // BugContest_Delete restores the party and transfers the caught
                // Pokemon only while this flag remains set.
                bugContest->state &= ~BUG_CONTEST_CAUGHT_POKEMON;
            } else {
                captureRules->bugContestPokemonKept = TRUE;
            }
        }

        BugContest_Delete(fieldSystem->bugContest);
        fieldSystem->bugContest = NULL;
#ifdef IMPLEMENT_PERMANENT_DEATH
        // BugContest_Delete has now restored the persistent party. Processing
        // earlier would remove from the temporary one-Pokemon contest party.
        PermanentDeath_ProcessPartyAfterBattle(fieldSystem->savedata);
#endif
        sub_02093070(fieldSystem);
#ifdef IMPLEMENT_PERMANENT_DEATH
        // Defer until the Contest script has completely released the field.
        PermanentDeath_ScheduleNotification(fieldSystem);
#endif
    }

    return FALSE;
}

#endif
