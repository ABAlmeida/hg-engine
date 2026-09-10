#include "../include/config.h"
#include "../include/legendary_sanctuary.h"
#include "../include/constants/species.h"

#ifdef IMPLEMENT_LEGENDARY_SANCTUARY

// Replacement pools: a later stage does not inherit entries from an earlier
// stage. Every distinct eligible species in a selected stage has equal
// probability; duplicate species rows are ignored after the first.
const SanctuaryEncounter
    gSanctuaryEncounterPools[SANCTUARY_STAGE_COUNT][SANCTUARY_MAX_POOL_SIZE] = {
        [0] = {
            { SPECIES_ARTICUNO, 40, 40 },
            { SPECIES_ZAPDOS, 40, 40 },
            { SPECIES_MOLTRES, 40, 40 },
            { SPECIES_RAIKOU, 40, 40 },
            { SPECIES_ENTEI, 40, 40 },
            { SPECIES_SUICUNE, 40, 40 },
            { SPECIES_REGIROCK, 40, 40 },
            { SPECIES_REGICE, 40, 40 },
            { SPECIES_REGISTEEL, 40, 40 },
            { SPECIES_UXIE, 40, 40 },
            { SPECIES_MESPRIT, 40, 40 },
            { SPECIES_AZELF, 40, 40 },
        },
        [1] = { { SPECIES_NONE, 0, 0 } },
        [2] = { { SPECIES_NONE, 0, 0 } },
        [3] = { { SPECIES_NONE, 0, 0 } },
    };

#endif
