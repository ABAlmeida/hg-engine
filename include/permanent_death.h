#ifndef PERMANENT_DEATH_H
#define PERMANENT_DEATH_H

#include "types.h"

typedef struct BattleSetup BattleSetup;
typedef struct FieldSystem FieldSystem;
typedef struct SaveData SaveData;
typedef struct TaskManager TaskManager;

void PermanentDeath_ProcessPartyAfterBattle(SaveData *saveData);
BOOL PermanentDeath_HasPendingNotification(void);
u8 PermanentDeath_GetRecoveredPartySlot(void);
BOOL PermanentDeath_ShouldEndRun(SaveData *saveData);
void PermanentDeath_ScheduleNotification(FieldSystem *fieldSystem);
void PermanentDeath_FinishNotification(BOOL endRun);
void LONG_CALL PermanentDeath_BlackoutWarpOrSkip(TaskManager *taskManager, const void *deathWarp);
void LONG_CALL PermanentDeath_BlackoutPrintMessage(void *environment, s32 messageId, u8 x, u8 y);
void LONG_CALL PermanentDeath_BlackoutRestoreOrEnd(TaskManager *taskManager);
void LONG_CALL PermanentDeath_PostBattleCommitHook(BattleSetup *setup, FieldSystem *fieldSystem);
void LONG_CALL PermanentDeath_CallTaskFadeFromBlack(TaskManager *taskManager);

#endif // PERMANENT_DEATH_H
