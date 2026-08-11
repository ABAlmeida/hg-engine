#ifndef HEARTLESS_GOLD_TRAINER_AI_H
#define HEARTLESS_GOLD_TRAINER_AI_H

#include "types.h"

struct BattleStruct;
struct BattleSystem;

BOOL TrainerAI_UsesStrategicLayer(u32 aiFlags);

void FairTrainerAI_Reset(struct BattleSystem *bsys);
void FairTrainerAI_ObserveAbility(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, u16 ability);
void FairTrainerAI_ObserveHeldItem(struct BattleSystem *bsys, struct BattleStruct *ctx, int battlerId, u16 item);

u8 FairTrainerAI_PickCommand(struct BattleSystem *bsys, u8 battlerId);
u8 FairTrainerAI_PickMove(struct BattleSystem *bsys, u8 battlerId);

u8 LONG_CALL TrainerAI_PickCommand_Original(struct BattleSystem *bsys, u8 battlerId);
u8 LONG_CALL TrainerAI_PickMove_Original(struct BattleSystem *bsys, u8 battlerId);

#endif
