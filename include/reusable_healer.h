#ifndef HEARTLESS_GOLD_REUSABLE_HEALER_H
#define HEARTLESS_GOLD_REUSABLE_HEALER_H

#include "types.h"

struct ItemCheckUseData;
struct ItemMenuUseData;
struct Party;

void LONG_CALL ReusableHealer_HealParty(struct Party *party);
void LONG_CALL ItemMenuUseFunc_ReusableHealer(
    struct ItemMenuUseData *data,
    const struct ItemCheckUseData *checkData);

#endif
