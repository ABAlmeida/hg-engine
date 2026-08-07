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
        || species > MAX_MON_NUM
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

    captureRules = CaptureRules_GetSave(saveData);
    if (!CaptureRules_IsValidArea(mapSection)) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_AREA);
        return;
    }

    // A duplicate still consumes Safari's opportunity when it is unused, but
    // always reports the more specific duplicate restriction afterward.
    if (isDuplicate) {
        CaptureRules_ConsumeArea(captureRules, mapSection);
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_DUPLICATE);
        return;
    }

    if (CaptureRules_IsAreaConsumed(captureRules, mapSection)) {
        CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_BLOCKED_AREA);
        return;
    }

    CaptureRules_ConsumeArea(captureRules, mapSection);
    CaptureRules_SetEncounterPermission(CAPTURE_PERMISSION_ALLOWED_STANDARD);
}

void LONG_CALL CaptureRules_InitializeSpecialBattle(struct BattleSystem *battleSystem)
{
    struct PartyPokemon *wildMon;
    struct SaveData *saveData;

    // Ordinary encounters were evaluated by the field-generation hook. Safari
    // uses a different generator, so initialize it after the battle party loads.
    if (CaptureRules_GetEncounterPermission() != CAPTURE_PERMISSION_UNRESTRICTED
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
