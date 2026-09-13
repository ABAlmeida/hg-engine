#ifndef HEARTLESS_GOLD_LAPTOP_H
#define HEARTLESS_GOLD_LAPTOP_H

#include "types.h"

struct ItemCheckUseData;
struct ItemFieldUseData;
struct ItemMenuUseData;

void LONG_CALL ItemMenuUseFunc_Laptop(
    struct ItemMenuUseData *data,
    const struct ItemCheckUseData *checkData);
BOOL LONG_CALL ItemFieldUseFunc_Laptop(struct ItemFieldUseData *data);
u32 LONG_CALL ItemCheckUseFunc_Laptop(const struct ItemCheckUseData *checkData);

#endif
