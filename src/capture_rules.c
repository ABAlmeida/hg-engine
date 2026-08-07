#include "../include/capture_rules.h"

#include "../include/config.h"

#ifdef IMPLEMENT_CAPTURE_RULES

_Static_assert(CAPTURE_AREA_WORD_COUNT == 8, "Capture area bitset must cover map sections 0 through 234");
_Static_assert(sizeof(CaptureRulesSave) == 36, "CaptureRulesSave layout changed");

// This permission crosses the field-to-battle overlay transition, so it must
// remain in the permanently loaded extension rather than either child overlay.
static CapturePermission sEncounterPermission;

void CaptureRules_Init(CaptureRulesSave *captureRules)
{
    memset(captureRules, 0, sizeof(*captureRules));
}

void LONG_CALL CaptureRules_ResetEncounterState(void)
{
    sEncounterPermission = CAPTURE_PERMISSION_UNRESTRICTED;
}

void LONG_CALL CaptureRules_SetEncounterPermission(CapturePermission permission)
{
    sEncounterPermission = permission;
}

CapturePermission LONG_CALL CaptureRules_GetEncounterPermission(void)
{
    return sEncounterPermission;
}

#endif
