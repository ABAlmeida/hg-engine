.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "asm/include/abilities.inc"
.include "asm/include/species.inc"

// GiveEgg receives the offset within the existing gift-location group.
// METLOC_JOHTO is 2004 in HGSS, so its verified MAPLOC value is 4.
METLOC_JOHTO_OFFSET equ 4

.create "build/a012/2_864", 0

scrdef violet_trade_house_unused
scrdef violet_type_egg_gift
scrdef violet_trade_house_advice
scrdef_end

violet_trade_house_unused:
    end

violet_type_egg_gift:
    play_se SEQ_SE_DP_SELECT
    lockall
    faceplayer
    goto_if_set FLAG_TRADE_VIOLET_CITY_BELLSPROUT_ONIX, violet_type_egg_already_received
    npc_msg 1
    touchscreen_menu_hide
    menu_init 1, 1, 0, 1, VAR_SPECIAL_RESULT
    menu_item_add 2, 255, 0
    menu_item_add 3, 255, 1
    menu_item_add 4, 255, 2
    menu_item_add 5, 255, 3
    menu_exec
    touchscreen_menu_show
    compare VAR_SPECIAL_RESULT, 0
    goto_if_eq violet_type_egg_dark
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq violet_type_egg_steel
    compare VAR_SPECIAL_RESULT, 2
    goto_if_eq violet_type_egg_ice
    goto violet_type_egg_cancel

violet_type_egg_dark:
    setvar VAR_SPECIAL_x8004, SPECIES_SANDILE
    SetGiftEggConfig VAR_SPECIAL_x8007, 25, ABILITY_MOXIE
    goto violet_type_egg_give

violet_type_egg_steel:
    setvar VAR_SPECIAL_x8004, SPECIES_TINKATINK
    SetGiftEggConfig VAR_SPECIAL_x8007, 25, ABILITY_MOLD_BREAKER
    goto violet_type_egg_give

violet_type_egg_ice:
    setvar VAR_SPECIAL_x8004, SPECIES_SWINUB
    SetGiftEggConfig VAR_SPECIAL_x8007, 25, ABILITY_THICK_FAT

violet_type_egg_give:
    setvar VAR_SPECIAL_x8005, METLOC_JOHTO_OFFSET
    setvar VAR_SPECIAL_x8006, FLAG_TRADE_VIOLET_CITY_BELLSPROUT_ONIX
    callstd std_configured_egg_gift
    compare VAR_SPECIAL_RESULT, 0
    goto_if_eq violet_type_egg_party_full
    buffer_players_name 0
    play_fanfare SEQ_ME_TAMAGO_GET
    npc_msg 8
    wait_fanfare
    wait_button
    closemsg
    releaseall
    end

violet_type_egg_cancel:
    npc_msg 6
    wait_button
    closemsg
    releaseall
    end

violet_type_egg_party_full:
    npc_msg 7
    wait_button
    closemsg
    releaseall
    end

violet_type_egg_already_received:
    npc_msg 9
    wait_button
    closemsg
    releaseall
    end

violet_trade_house_advice:
    simple_npc_msg 0
    end

.close
