#ifndef HEARTLESS_GOLD_BAIT_H
#define HEARTLESS_GOLD_BAIT_H

#include "types.h"

struct ItemCheckUseData;
struct ItemMenuUseData;

void LONG_CALL ItemMenuUseFunc_Bait(struct ItemMenuUseData *data, const struct ItemCheckUseData *checkData);
u32 LONG_CALL ItemCheckUseFunc_Bait(const struct ItemCheckUseData *checkData);

BOOL LONG_CALL Bait_IsGeneratingEncounter(void);
BOOL LONG_CALL Bait_ShouldForceShiny(void);

#endif
