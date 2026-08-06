#ifndef POKEHEARTGOLD_STAT_TRAINING_ITEMS_H
#define POKEHEARTGOLD_STAT_TRAINING_ITEMS_H

#include "types.h"

struct PartyPokemon;

BOOL StatTrainingItem_IsHandled(u16 itemId);
BOOL StatTrainingItem_CanUseOnMon(struct PartyPokemon *mon, u16 itemId);
// Called from the separately linked party-menu overlay; preserve Thumb state.
BOOL LONG_CALL StatTrainingItem_UseOnMon(
    struct PartyPokemon *mon, u16 itemId, u16 mapSection);

#endif // POKEHEARTGOLD_STAT_TRAINING_ITEMS_H
