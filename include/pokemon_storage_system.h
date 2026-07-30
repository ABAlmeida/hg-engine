#ifndef INCLUDE_POKEMON_STORAGE_SYSTEM_H
#define INCLUDE_POKEMON_STORAGE_SYSTEM_H

#include "config.h"
//#include "save.h"
#include "types.h"
#include "pokemon.h"
#include "constants/save.h"

#define BOX_ALL_MODIFIED_FLAG ((u32)((1<<NUM_PC_BOXES)-1))

#define MONS_PER_BOX 30
#define BOX_NAME_LENGTH 20
#define DEFAULT_WALLPAPER_MIN           0
#define WALLPAPER_FOREST                0
#define WALLPAPER_CITY                  1
#define WALLPAPER_DESERT                2
#define WALLPAPER_SAVANNA               3
#define WALLPAPER_CRAG                  4
#define WALLPAPER_VOLCANO               5
#define WALLPAPER_SNOW                  6
#define WALLPAPER_CAVE                  7
#define WALLPAPER_BEACH                 8
#define WALLPAPER_SEAFLOOR              9
#define WALLPAPER_RIVER                10
#define WALLPAPER_SKY                  11
#define WALLPAPER_POKE_CENTER          12
#define WALLPAPER_MACHINE              13
#define WALLPAPER_CHECKS               14
#define WALLPAPER_SIMPLE               15
#define DEFAULT_WALLPAPER_MAX          16

#define BONUS_WALLPAPER_MIN            32
#define WALLPAPER_HEART                32
#define WALLPAPER_SOUL                 33
#define WALLPAPER_BIG_BROTHER          34
#define WALLPAPER_POKEATHLON           35
#define WALLPAPER_TRIO                 36
#define WALLPAPER_SPIKY_PIKA           37
#define WALLPAPER_KIMONO_GIRL          38
#define WALLPAPER_REVIVAL              39
#define BONUS_WALLPAPER_MAX            40

#define NUM_DEFAULT_WALLPAPER          (DEFAULT_WALLPAPER_MAX-DEFAULT_WALLPAPER_MIN)
#define NUM_BONUS_WALLPAPER            (BONUS_WALLPAPER_MAX-BONUS_WALLPAPER_MIN)


typedef struct PACKED PokemonStorageSystem PCStorage;

typedef struct PC_Box {
    struct BoxPokemon mons[MONS_PER_BOX];
    u8 unk_FF0[16]; // padding
} PC_BOX;

struct PACKED PokemonStorageSystem {
    /* 00000 */ PC_BOX boxes[NUM_PC_BOXES];
    /* 1E000 */ int curBox;
    /* 1E004 */ u32 boxModifiedFlag;
    /* 1E008 */ u16 box_names[NUM_PC_BOXES][BOX_NAME_LENGTH];
    /* 1E4B8 */ u8 wallpapers[NUM_PC_BOXES];
    /* 1E4D6 */ u8 unlockedWallpapers;
    /* 1E4D7 */ u8 filler_1E4D7[0x11];
};

_Static_assert(sizeof(PCStorage) == 0x1E4E8, "Expanded PCStorage layout changed");
_Static_assert(offsetof(PCStorage, curBox) == 0x1E000, "PCStorage active-box offset changed");
_Static_assert(offsetof(PCStorage, boxModifiedFlag) == 0x1E004, "PCStorage modified-box offset changed");

BOOL PCStorage_PlaceMonInBoxFirstEmptySlot(PCStorage* storage, u32 boxno, struct BoxPokemon *boxMon);
void PCStorage_SetBoxModified(PCStorage *storage, u8 boxno);
void PCStorage_DeleteBoxMonByIndexPair(PCStorage *storage, u32 boxno, u32 slotno);
struct BoxPokemon *PCStorage_GetMonByIndexPair(PCStorage *storage, u32 boxno, u32 slotno);

#endif // INCLUDE_POKEMON_STORAGE_SYSTEM_H
