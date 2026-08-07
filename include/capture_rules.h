#ifndef CAPTURE_RULES_H
#define CAPTURE_RULES_H

#include "constants/map_sections.h"
#include "types.h"

#define CAPTURE_AREA_BITS_PER_WORD 32
#define CAPTURE_AREA_WORD_COUNT    ((MAPSEC_COUNT + CAPTURE_AREA_BITS_PER_WORD - 1) / CAPTURE_AREA_BITS_PER_WORD)

typedef struct CaptureRulesSave {
    // One bit per displayed map section, shared by ordinary and Safari encounters.
    u32 consumedAreas[CAPTURE_AREA_WORD_COUNT];
    // Set only after a Bug-Catching Contest Pokemon is actually retained.
    u8 bugContestPokemonKept;
    u8 padding[3];
} CaptureRulesSave;

// Per-battle result of evaluating the generated wild Pokemon. This state is
// transient; only consumedAreas and Bug Contest retention are saved.
typedef enum CapturePermission {
    CAPTURE_PERMISSION_UNRESTRICTED,
    CAPTURE_PERMISSION_ALLOWED_STANDARD,
    CAPTURE_PERMISSION_ALLOWED_SHINY,
    CAPTURE_PERMISSION_BLOCKED_AREA,
    CAPTURE_PERMISSION_BLOCKED_DUPLICATE,
} CapturePermission;

struct SaveData;
struct BATTLE_PARAM;
struct BattleSystem;
struct PartyPokemon;
typedef struct SCRIPTCONTEXT SCRIPTCONTEXT;

void CaptureRules_Init(CaptureRulesSave *captureRules);
void LONG_CALL CaptureRules_ResetEncounterState(void);
void LONG_CALL CaptureRules_SetEncounterPermission(CapturePermission permission);
CapturePermission LONG_CALL CaptureRules_GetEncounterPermission(void);
void LONG_CALL CaptureRules_GenerateOrdinaryEncounter(
    u16 species,
    u8 level,
    int battler,
    BOOL forceOnePerfectIV,
    void *encounterGen,
    struct PartyPokemon *leadMon,
    struct BATTLE_PARAM *battleParam);
void LONG_CALL CaptureRules_InitializeSpecialBattle(struct BattleSystem *battleSystem);
BOOL LONG_CALL CaptureRules_ScrCmdBugContestAction(SCRIPTCONTEXT *ctx);

#endif
