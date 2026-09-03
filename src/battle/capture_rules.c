#include "../../include/capture_rules.h"

#include "../../include/battle.h"
#include "../../include/config.h"
#include "../../include/pokedex.h"
#include "../../include/pokemon.h"
#include "../../include/save.h"
#include "../../include/constants/species.h"

#ifdef IMPLEMENT_CAPTURE_RULES

static BOOL CaptureRules_IsValidArea(u32 mapSection)
{
    return mapSection < MAPSEC_COUNT;
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

    return (captureRules->consumedAreas[mapSection / CAPTURE_AREA_BITS_PER_WORD]
               & (1U << (mapSection % CAPTURE_AREA_BITS_PER_WORD)))
        != 0;
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
    if (species == SPECIES_NONE
        || species > SPECIES_MAX_MON_NUM
        || (species >= SPECIES_EGG && species < SPECIES_VICTINI)) {
        return FALSE;
    }

    return Pokedex_GetCaughtFlag(SaveData_GetDexPtr(saveData), species);
}

static void CaptureRules_EvaluateSafariEncounter(
    struct SaveData *saveData,
    u32 mapSection,
    struct PartyPokemon *wildMon)
{
    CaptureRulesSave *captureRules;
    BOOL isDuplicate;
    u16 species;

    if (saveData == NULL || wildMon == NULL) {
        return;
    }

    species = GetMonData(wildMon, MON_DATA_SPECIES, NULL);
    isDuplicate = CaptureRules_IsSpeciesDuplicate(saveData, species);

    // Only non-duplicate shinies bypass Safari's saved opportunity.
    if (MonIsShiny(wildMon) && !isDuplicate) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_ALLOWED_SHINY);
        return;
    }

    // Match ordinary encounters: a duplicate is never an eligible encounter,
    // so it reports the specific restriction without consuming the area.
    if (isDuplicate) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_DUPLICATE);
        return;
    }

    captureRules = CaptureRules_GetSave(saveData);
    if (!CaptureRules_IsValidArea(mapSection)) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_AREA);
        return;
    }

    if (CaptureRules_IsAreaConsumed(captureRules, mapSection)) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_AREA);
        return;
    }

    CaptureRules_ConsumeArea(captureRules, mapSection);
    CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_ALLOWED_STANDARD);
}

void LONG_CALL CaptureRules_InitializeBattle(struct BattleSystem *battleSystem)
{
    struct PartyPokemon *wildMon;
    struct SaveData *saveData;
    CapturePermission permission = CaptureRules_GetEncounterPermission();

    // Field generation has already approved this ordinary encounter. Commit
    // its area here, once the encounter has actually become a battle. This
    // still makes defeat and fleeing consume the opportunity, but a failed
    // fishing timing prompt never reaches this point.
    if (permission == CAPTURE_PERMISSION_ALLOWED_STANDARD) {
        saveData = SaveBlock2_get();
        CaptureRules_ConsumeArea(
            CaptureRules_GetSave(saveData),
            BattleWorkPlaceIDGet(battleSystem));
        return;
    }

    // Safari uses a different generator, so evaluate it after the battle party
    // has loaded.
    if (permission != CAPTURE_PERMISSION_UNRESTRICTED
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

#endif
