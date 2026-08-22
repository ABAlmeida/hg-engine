#include "constants/battle_constants.h"
#include "constants/battle_message_constants.h"
.include "battle_commands.inc"

.data

_Start:
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_TRAINER, _TrainerEncounter
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_SAFARI, _SafariEncounter
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_PAL_PARK, _PalParkEncounter
    SetPokemonEncounter BATTLER_CATEGORY_ENEMY
    SetTrainerEncounter BATTLER_CATEGORY_PLAYER
    PlayEncounterAnimation
    WaitTime 122
    HealthbarSlideInDelay BATTLER_CATEGORY_ENEMY
    Wait
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_SYS_STATUS, BATTLE_SPECIAL_DISTORTION_WORLD, _DistortionWorldMessage
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_SYS_STATUS, BATTLE_SPECIAL_LEGENDARY, _LegendaryMessage
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_SYS_STATUS, BATTLE_SPECIAL_FIRST_RIVAL, _FirstEncounterMessage
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_SYS_STATUS, BATTLE_SPECIAL_HONEY_TREE, _HoneyTreeMessage
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_TOTEM, _TotemEncounter
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_DOUBLES, _WildDoublesMessage
    // You encountered a wild {0}!
    PrintGlobalMessage 965, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    GoTo _CaptureAvailabilityMessage

_FirstEncounterMessage:
    // Whoa! A wild {0} came charging!
    PrintGlobalMessage 1167, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    GoTo _CaptureAvailabilityMessage

_HoneyTreeMessage:
    // A wild {0} appeared from the tree you slathered with Honey!
    PrintGlobalMessage 968, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    GoTo _CaptureAvailabilityMessage

_LegendaryMessage:
    // {0} appeared!
    PrintGlobalMessage 1246, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    GoTo _CaptureAvailabilityMessage

_DistortionWorldMessage:
    // The Distortion World’s {0} appeared!
    PrintGlobalMessage 1268, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    GoTo _CaptureAvailabilityMessage

_WildDoublesMessage:
    // A wild {0} and {1} appeared!
    PrintGlobalMessage 967, TAG_NICKNAME_NICKNAME, BATTLER_CATEGORY_ENEMY_SLOT_1, BATTLER_CATEGORY_ENEMY_SLOT_2
    GoTo _CaptureAvailabilityMessage

_TotemEncounter:
    // You are challenged by {0}!
    PrintGlobalMessage 1757, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    Wait
    WaitButtonABTime 30
    MakeTotem BATTLER_ENEMY, _SendOutPokemonMessage
    PlayBattleAnimation BATTLER_CATEGORY_ENEMY, BATTLE_ANIMATION_STAT_BOOST
    Wait
    PrintBufferedMessage
    GoTo _SendOutPokemonMessage

_CaptureAvailabilityMessage:
    Wait
    WaitButtonABTime 30
    // tempData is initialized immediately before this encounter script. Zero
    // means the current battle does not offer a usable capture command.
    CompareVarToValue OPCODE_EQU, BSCRIPT_VAR_TEMP_DATA, 0, _PrintSendOutPokemonMessage
    // This Pokémon can be captured.
    PrintGlobalMessage BATTLE_MSG_CAPTURE_ALLOWED, TAG_NONE
    Wait
    WaitButtonABTime 120
    GoTo _PrintSendOutPokemonMessage

_SendOutPokemonMessage:
    Wait
    WaitButtonABTime 30
_PrintSendOutPokemonMessage:
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_MULTI, _SendOutPokemonMessage_Multi
    CompareVarToValue OPCODE_EQU, BSCRIPT_VAR_BATTLE_TYPE, BATTLE_TYPE_DOUBLES, _SendOutPokemonMessage_Doubles
    // Go! {0}!
    PrintMessage 979, TAG_NICKNAME, BATTLER_CATEGORY_PLAYER
    GoTo _SendOutPokemon

_SendOutPokemonMessage_Doubles:
    // Go! {0} and {1}!
    PrintMessage 978, TAG_NICKNAME_NICKNAME, BATTLER_CATEGORY_PLAYER_SLOT_1, BATTLER_CATEGORY_PLAYER_SLOT_2
    GoTo _SendOutPokemon

_SendOutPokemonMessage_Multi:
    PrintFirstSendOutMessage BATTLER_CATEGORY_PLAYER

_SendOutPokemon:
    SpriteToOAM BATTLER_CATEGORY_ENEMY
    Wait
    ThrowPokeball BATTLER_CATEGORY_PLAYER, BTLSCR_THROW_POKE_BALL
    PokemonSlideIn BATTLER_CATEGORY_PLAYER
    WaitTime 96
    HealthbarSlideInDelay BATTLER_CATEGORY_PLAYER
    Wait
    OAMToSprite BATTLER_CATEGORY_ENEMY
    Wait
    GoTo _Cleanup

_TrainerEncounter:
    CompareVarToValue OPCODE_FLAG_SET, BSCRIPT_VAR_BATTLE_SYS_STATUS, BATTLE_SPECIAL_RECORDED, _RecordedBattle
    PlayEncounterAnimation
    SetTrainerEncounter BATTLER_CATEGORY_ALL
    WaitTime 96
    LoadPartyGaugeGraphics
    ShowBattleStartPartyGauge BATTLER_CATEGORY_ENEMY
    ShowBattleStartPartyGauge BATTLER_CATEGORY_PLAYER
    PrintEncounterMessage BATTLER_CATEGORY_ENEMY
    Wait
    WaitButtonABTime 30
    PrintFirstSendOutMessage BATTLER_CATEGORY_ENEMY
    HideBattleStartPartyGauge BATTLER_CATEGORY_ENEMY
    ThrowPokeball BATTLER_CATEGORY_ENEMY, BTLSCR_THROW_POKE_BALL
    PokemonSlideIn BATTLER_CATEGORY_ENEMY
    WaitTime 112
    HealthbarSlideInDelay BATTLER_CATEGORY_ENEMY
    Wait
    PrintFirstSendOutMessage BATTLER_CATEGORY_PLAYER
    HideBattleStartPartyGauge BATTLER_CATEGORY_PLAYER
    ThrowPokeball BATTLER_CATEGORY_PLAYER, BTLSCR_THROW_POKE_BALL
    PokemonSlideIn BATTLER_CATEGORY_PLAYER
    WaitTime 96
    HealthbarSlideInDelay BATTLER_CATEGORY_PLAYER
    Wait
    FreePartyGaugeGraphics
    GoTo _Cleanup

_RecordedBattle:
    PlayEncounterAnimation
    SetTrainerEncounter BATTLER_CATEGORY_ALL
    WaitTime 96
    LoadPartyGaugeGraphics
    ShowBattleStartPartyGauge BATTLER_CATEGORY_ENEMY
    ShowBattleStartPartyGauge BATTLER_CATEGORY_PLAYER
    PrintEncounterMessage BATTLER_CATEGORY_ENEMY
    Wait
    WaitButtonABTime 30
    PrintFirstSendOutMessage BATTLER_CATEGORY_PLAYER
    HideBattleStartPartyGauge BATTLER_CATEGORY_PLAYER
    ThrowPokeball BATTLER_CATEGORY_PLAYER, BTLSCR_THROW_POKE_BALL
    PokemonSlideIn BATTLER_CATEGORY_PLAYER
    WaitTime 96
    HealthbarSlideInDelay BATTLER_CATEGORY_PLAYER
    Wait
    PrintFirstSendOutMessage BATTLER_CATEGORY_ENEMY
    HideBattleStartPartyGauge BATTLER_CATEGORY_ENEMY
    ThrowPokeball BATTLER_CATEGORY_ENEMY, BTLSCR_THROW_POKE_BALL
    PokemonSlideIn BATTLER_CATEGORY_ENEMY
    WaitTime 112
    HealthbarSlideInDelay BATTLER_CATEGORY_ENEMY
    Wait
    FreePartyGaugeGraphics
    GoTo _Cleanup

_SafariEncounter:
    SetPokemonEncounter BATTLER_CATEGORY_ENEMY
    SetTrainerEncounter BATTLER_CATEGORY_PLAYER
    PlayEncounterAnimation
    WaitTime 122
    HealthbarSlideInDelay BATTLER_CATEGORY_ENEMY
    Wait
    // You encountered a wild {0}!
    PrintGlobalMessage 965, TAG_NICKNAME, BATTLER_CATEGORY_ENEMY
    Wait
    HealthbarSlideIn BATTLER_CATEGORY_PLAYER
    WaitButtonABTime 7
    Wait
    CompareVarToValue OPCODE_EQU, BSCRIPT_VAR_TEMP_DATA, 0, _Cleanup
    // This Pokémon can be captured.
    PrintGlobalMessage BATTLE_MSG_CAPTURE_ALLOWED, TAG_NONE
    Wait
    WaitButtonABTime 120
    GoTo _Cleanup

_PalParkEncounter:
    SetPokemonEncounter BATTLER_CATEGORY_ENEMY
    SetTrainerEncounter BATTLER_CATEGORY_PLAYER
    PlayEncounterAnimation
    WaitTime 122
    HealthbarSlideInDelay BATTLER_CATEGORY_ENEMY
    Wait
    // Wow! {0}’s {1} is drawing close!
    PrintGlobalMessage 1219, TAG_TRNAME_NICKNAME, BATTLER_CATEGORY_ENEMY, BATTLER_CATEGORY_ENEMY
    Wait
    HealthbarSlideIn BATTLER_CATEGORY_PLAYER
    WaitButtonABTime 7
    Wait

_Cleanup:
    SetBattleBackground
    End
