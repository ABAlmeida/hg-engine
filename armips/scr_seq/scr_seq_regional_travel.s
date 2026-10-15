.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

// The first voyage's arrival/disembarkation sequence advances 0x40CB to 7.
// Reuse that persistent state, including voyages completed on existing saves.
SS_AQUA_FIRST_VOYAGE_COMPLETE equ 7
OLIVINE_BOARDING equ 0x138
VERMILION_BOARDING equ 0x29
OLIVINE_PORT_END equ filesize("build/a012/2_152")
VERMILION_PORT_END equ filesize("build/a012/2_154")

.if readu32("build/a012/2_152", 0) != OLIVINE_BOARDING - 4
    .error "Unexpected Olivine boarding entry"
.endif
.if readu16("build/a012/2_152", 0x1FB) != 484
    .error "Unexpected Olivine weekday check"
.endif
.open "build/a012/2_152", 0
.org 0
scrdef olivine_daily_ship
.org 0x1FB
    goto 0x29B
.org OLIVINE_PORT_END
olivine_daily_ship:
    // A north-facing interaction is the original route off the ship: never
    // apply the boarding lock to passengers disembarking at either port.
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_NORTH
    goto_if_eq OLIVINE_BOARDING
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq OLIVINE_BOARDING
    compare VAR_UNK_40CB, SS_AQUA_FIRST_VOYAGE_COMPLETE
    goto_if_lt OLIVINE_BOARDING
    simple_npc_msg 16
    end
.close

.if readu32("build/a012/2_154", 0) != VERMILION_BOARDING - 4
    .error "Unexpected Vermilion boarding entry"
.endif
.if readu16("build/a012/2_154", 0x4F) != 484
    .error "Unexpected Vermilion weekday check"
.endif
.open "build/a012/2_154", 0
.org 0
scrdef vermilion_daily_ship
.org 0x4F
    goto 0xEF
.org VERMILION_PORT_END
vermilion_daily_ship:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_NORTH
    goto_if_eq VERMILION_BOARDING
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq VERMILION_BOARDING
    compare VAR_UNK_40CB, SS_AQUA_FIRST_VOYAGE_COMPLETE
    goto_if_lt VERMILION_BOARDING
    simple_npc_msg 4
    end
.close

// Preserve each station's interaction and ticket scripts, including the
// original Pass checks and Power Plant-dependent guard/coordinate setup.
ORIGINAL_SAFFRON_TRAIN_GUARD equ readu32("build/a012/2_834", 0) + 4
ORIGINAL_SAFFRON_TRAIN_TICKET equ readu32("build/a012/2_834", 0x10) + 0x14
SAFFRON_TRAIN_END equ filesize("build/a012/2_834")
.if readu16("build/a012/2_834", 0x24) != SCRDEF_END_CONSTANT || readu16("build/a012/2_834", ORIGINAL_SAFFRON_TRAIN_TICKET) != 609
    .error "Unexpected Saffron train script entries"
.endif
.open "build/a012/2_834", 0
.org 0
scrdef saffron_train_guard
.org 0x10
scrdef saffron_train_ticket
.org SAFFRON_TRAIN_END
saffron_train_guard:
    goto_if_unset FLAG_RETURNED_TO_JOHTO_BY_MAGNET_TRAIN, ORIGINAL_SAFFRON_TRAIN_GUARD
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq ORIGINAL_SAFFRON_TRAIN_GUARD
    simple_npc_msg 19
    end
saffron_train_ticket:
    goto_if_unset FLAG_RETURNED_TO_JOHTO_BY_MAGNET_TRAIN, ORIGINAL_SAFFRON_TRAIN_TICKET
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq ORIGINAL_SAFFRON_TRAIN_TICKET
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 19
    wait_button
    closemsg
    // Use the original no-Pass return path, including follower handling.
    scrcmd_602 0
    wait_following_pokemon_movement
    scrcmd_604 55
    apply_movement obj_player, 0x184
    wait_movement
    wait_following_pokemon_movement
    scrcmd_602 1
    scrcmd_604 48
    releaseall
    end
.close

ORIGINAL_GOLDENROD_TRAIN_GUARD equ readu32("build/a012/2_893", 0) + 4
ORIGINAL_GOLDENROD_TRAIN_TICKET equ readu32("build/a012/2_893", 4) + 8
GOLDENROD_TRAIN_END equ filesize("build/a012/2_893")
.if readu16("build/a012/2_893", 0x20) != SCRDEF_END_CONSTANT || readu16("build/a012/2_893", ORIGINAL_GOLDENROD_TRAIN_TICKET) != 609
    .error "Unexpected Goldenrod train script entries"
.endif
.open "build/a012/2_893", 0
.org 0
scrdef goldenrod_train_guard
.org 4
scrdef goldenrod_train_ticket
.org GOLDENROD_TRAIN_END
goldenrod_train_guard:
    goto_if_unset FLAG_RETURNED_TO_JOHTO_BY_MAGNET_TRAIN, ORIGINAL_GOLDENROD_TRAIN_GUARD
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq ORIGINAL_GOLDENROD_TRAIN_GUARD
    simple_npc_msg 10
    end
goldenrod_train_ticket:
    goto_if_unset FLAG_RETURNED_TO_JOHTO_BY_MAGNET_TRAIN, ORIGINAL_GOLDENROD_TRAIN_TICKET
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq ORIGINAL_GOLDENROD_TRAIN_TICKET
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 10
    wait_button
    closemsg
    scrcmd_602 0
    wait_following_pokemon_movement
    scrcmd_604 55
    apply_movement obj_player, 0x144
    wait_movement
    wait_following_pokemon_movement
    scrcmd_602 1
    scrcmd_604 48
    releaseall
    end
.close

// Set completion at the end of Goldenrod's arrival scene, after the actual
// journey, warp, arrival message, and walk off the platform have completed.
GOLDENROD_ARRIVAL_END equ filesize("build/a012/2_894")
.if readu16("build/a012/2_894", 0x83) != 604 || readu16("build/a012/2_894", 0x85) != 48 || readu16("build/a012/2_894", 0x87) != 97
    .error "Unexpected Goldenrod train arrival completion"
.endif
.open "build/a012/2_894", 0
.org 0x83
    goto goldenrod_train_arrival_complete
.org GOLDENROD_ARRIVAL_END
goldenrod_train_arrival_complete:
    scrcmd_604 48
    setflag FLAG_RETURNED_TO_JOHTO_BY_MAGNET_TRAIN
    releaseall
    end
.close
