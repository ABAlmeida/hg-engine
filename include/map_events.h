#ifndef POKEHEARTGOLD_MAP_EVENTS_H
#define POKEHEARTGOLD_MAP_EVENTS_H

#include "encounter.h"

typedef struct FieldSystem FieldSystem;

EncounterData *LONG_CALL MapEvents_GetLoadedEncTable(FieldSystem *fieldSystem);

#endif // POKEHEARTGOLD_MAP_EVENTS_H
