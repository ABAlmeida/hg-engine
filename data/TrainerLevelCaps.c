#include "../include/level_cap.h"

// This table is installed in ARC_CODE_ADDONS and indexed directly by trainer
// ID. Unlisted trainers retain the zero-initialized value and do not change the
// current level cap.
const u8 sTrainerLevelCaps[LEVEL_CAP_REWARD_TABLE_COUNT] = {
    // Silver fight 1
    [496] = 13, // Cyndaquil
    [497] = 13, // Totodile
    [495] = 13, // Chikorita

    [20] = 17, // Falkner
    [21] = 18, // Bugsy

    // Silver fight 2
    [1] = 19,   // Bayleef
    [266] = 19, // Quilava
    [269] = 19, // Croconaw

    [30] = 22, // Whitney

    // Silver fight 3
    [263] = 25, // Bayleef
    [267] = 25, // Quilava
    [270] = 25, // Croconaw

    [31] = 31, // Morty
    [34] = 35, // Chuck
    [33] = 35, // Jasmine
    [32] = 35, // Pryce

    // Silver fight 4
    [288] = 38, // Meganium
    [289] = 38, // Quilava
    [271] = 38, // Feraligatr

    [485] = 41, // Executive Archer
    [35] = 41,  // Clair

    // Silver fight 5
    [264] = 42, // Meganium
    [268] = 42, // Typhlosion
    [272] = 42, // Feraligatr

    [245] = 44, // Will
    [247] = 46, // Koga
    [418] = 47, // Bruno
    [246] = 50, // Karen
    [244] = 55, // Lance

    // Each Kanto leader remains an independent balancing point.
    [255] = 55, // Lt. Surge
    [258] = 55, // Sabrina
    [256] = 55, // Erika
    [257] = 55, // Janine
    [254] = 55, // Misty
    [253] = 55, // Brock

    // Silver fight 6
    [285] = 59, // Meganium
    [286] = 59, // Typhlosion
    [287] = 59, // Feraligatr

    [259] = 60,  // Blaine
    [261] = 86,  // Blue
    [260] = 100, // Red
};
