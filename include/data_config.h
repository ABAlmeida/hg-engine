#ifndef DATA_CONFIG_H
#define DATA_CONFIG_H

// Settings in this file affect generated move/species archives. Keeping them
// separate prevents unrelated gameplay config changes from rebuilding the
// very large host-side data tables.

// Use a generation below 8 for no dexit restrictions; 0 disables moves marked
// as unimplemented.
#define DISALLOW_DEXIT_GEN 0

// Champions-specific move configurations. Set to 0 to use Scarlet/Violet values.
#define CHAMPIONS_POWER_CHANGES         1
#define CHAMPIONS_TYPE_CHANGES          1
#define CHAMPIONS_ACC_CHANGES           1
#define CHAMPIONS_PP_CHANGES            0
#define CHAMPIONS_EFFECT_CHANCE_CHANGES 1

#endif
