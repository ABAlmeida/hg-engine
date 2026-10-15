.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_NORTH equ 12
MOVE_WALK_SOUTH equ 13

// R05R0301: a barrier across the middle prevents crossing from either side.
// Every blocked crossing reverses the player's incoming movement, so they
// remain on their entrance's side without persistent arrival state.
.create "build/a012/2_182", 0
scrdef route_5_guard
scrdef route_5_intercept
scrdef route_5_init
scrdef_end

route_5_guard:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_5_guard_open
    simple_npc_msg 1
    end
route_5_guard_open:
    simple_npc_msg 0
    end

route_5_intercept:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_5_end
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_NORTH
    goto_if_eq route_5_from_saffron
    compare VAR_SPECIAL_RESULT, DIR_SOUTH
    goto_if_ne route_5_end
    call route_5_stop_message
    apply_movement obj_player, route_5_return_north
    goto route_5_finish_turnback
route_5_from_saffron:
    call route_5_stop_message
    apply_movement obj_player, route_5_return_south
route_5_finish_turnback:
    wait_movement
    releaseall
route_5_end:
    end

route_5_stop_message:
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 1
    wait_button
    closemsg
    return

route_5_init:
    check_badge BADGE_SOUL, VAR_TEMP_x400F
    end
.align 4
route_5_return_north:
    step MOVE_WALK_NORTH, 1
    step_end
.align 4
route_5_return_south:
    step MOVE_WALK_SOUTH, 1
    step_end
.close

.create "build/a012/2_427", 0
map_script 2, 3
.byte 0
.align 4
.close

// Append entries 7 and 8 to R10, preserving its seven original scripts.
.create "build/rock_tunnel_gate_scripts", 0
scrdef rock_tunnel_blocker
scrdef rock_tunnel_init
scrdef_end

rock_tunnel_blocker:
    simple_npc_msg 10
    end

rock_tunnel_init:
    clearflag FLAG_MAPTEMP_03F
    check_badge BADGE_CASCADE, VAR_TEMP_x400F
    compare VAR_TEMP_x400F, 1
    goto_if_ne rock_tunnel_original_transition
    setflag FLAG_MAPTEMP_03F

rock_tunnel_original_transition:
    // Preserve R10's original type-2 initializer: Lt. Surge's phone-number
    // scene can appear from 09:00 through 11:59 after catching Zapdos.
    check_badge BADGE_EARTH, VAR_TEMP_x4004
    compare VAR_TEMP_x4004, 1
    goto_if_ne rock_tunnel_init_end
    goto_if_unset FLAG_CAUGHT_ZAPDOS, rock_tunnel_init_end
    check_registered_phone_number PHONE_CONTACT_LT__SURGE, VAR_TEMP_x4001
    compare VAR_TEMP_x4001, 1
    goto_if_eq rock_tunnel_init_end
    scrcmd_522 VAR_TEMP_x4000
    compare VAR_TEMP_x4000, 9
    goto_if_eq rock_tunnel_show_surge
    compare VAR_TEMP_x4000, 10
    goto_if_eq rock_tunnel_show_surge
    compare VAR_TEMP_x4000, 11
    goto_if_eq rock_tunnel_show_surge
    setflag FLAG_HIDE_ROUTE_10_LT_SURGE
    end
rock_tunnel_show_surge:
    clearflag FLAG_HIDE_ROUTE_10_LT_SURGE
rock_tunnel_init_end:
    end
.close

// Use the new transition initializer and retain the original on-load script
// (entry 4 / one-based 5), which handles Zapdos encounter cleanup.
.create "build/a012/2_436", 0
map_script 2, 9
map_script 4, 5
.byte 0
.align 4
.close
