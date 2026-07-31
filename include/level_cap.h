#ifndef LEVEL_CAP_H
#define LEVEL_CAP_H

#include "types.h"

#define LEVEL_CAP_INITIAL 7
#define LEVEL_CAP_MAX     100

// Number of direct-indexed trainer entries stored in the level-cap code table.
#define LEVEL_CAP_REWARD_TABLE_COUNT 498

typedef struct BattleSetup BattleSetup;

BOOL LONG_CALL SetNewLevelCap(u8 newCap);
void LONG_CALL LevelCap_ApplyTrainerVictory(const BattleSetup *setup);

#endif // LEVEL_CAP_H
