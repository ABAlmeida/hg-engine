#ifndef HEARTLESS_GOLD_ENCOUNTER_CHECK_H
#define HEARTLESS_GOLD_ENCOUNTER_CHECK_H

#include "types.h"

typedef struct FieldSystem FieldSystem;
typedef struct TaskManager TaskManager;

BOOL LONG_CALL FieldSystem_PerformSweetScentEncounterCheck(FieldSystem *fieldSystem, TaskManager *taskManager);

#endif // HEARTLESS_GOLD_ENCOUNTER_CHECK_H
