.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_NORTH equ 12

// Appended to R02R0301's two original entries during script packaging.
// Preserve the lab aide and redirect only the counter guard's event ID.
.create "build/route_2_gate_scripts", 0
scrdef route_2_guard
scrdef route_2_intercept
scrdef route_2_init
scrdef_end

route_2_guard:
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_2_guard_open
    simple_npc_msg 4
    end
route_2_guard_open:
    simple_npc_msg 3
    end

route_2_intercept:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_SOUTH
    goto_if_ne route_2_end
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_2_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 4
    wait_button
    closemsg
    apply_movement obj_player, route_2_turn_back
    wait_movement
    releaseall
route_2_end:
    end

route_2_init:
    // Temporary unlock requirement: replace this and the interaction checks
    // together when the final story milestone is decided.
    check_badge BADGE_RISING, VAR_TEMP_x400F
    end
.align 4
route_2_turn_back:
    step MOVE_WALK_NORTH, 1
    step_end
.close

// R02R0501: block southbound entry into the forest from Diglett's Cave's
// side of Route 2; northbound exits from the forest remain available.
.create "build/a012/2_174", 0
scrdef forest_north_guard
scrdef forest_north_intercept
scrdef forest_north_init
scrdef_end

forest_north_guard:
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq forest_north_guard_open
    simple_npc_msg 1
    end
forest_north_guard_open:
    simple_npc_msg 0
    end

forest_north_intercept:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_SOUTH
    goto_if_ne forest_north_end
    check_badge BADGE_RISING, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq forest_north_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 1
    wait_button
    closemsg
    apply_movement obj_player, forest_north_turn_back
    wait_movement
    releaseall
forest_north_end:
    end

forest_north_init:
    check_badge BADGE_RISING, VAR_TEMP_x400F
    end
.align 4
forest_north_turn_back:
    step MOVE_WALK_NORTH, 1
    step_end
.close

.create "build/a012/2_417", 0
map_script 2, 5
.byte 0
.align 4
.close

.create "build/a012/2_419", 0
map_script 2, 3
.byte 0
.align 4
.close

// Appended to Route 3's five original entries without altering its signs
// or meteorite scripts. Entry 5 is the worker; entry 6 initializes visibility.
.create "build/mt_moon_gate_scripts", 0
scrdef mt_moon_blocker
scrdef mt_moon_init
scrdef_end

mt_moon_blocker:
    simple_npc_msg 6
    end
mt_moon_init:
    clearflag FLAG_MAPTEMP_03F
    check_badge BADGE_BOULDER, VAR_TEMP_x400F
    compare VAR_TEMP_x400F, 1
    goto_if_ne mt_moon_init_end
    setflag FLAG_MAPTEMP_03F
mt_moon_init_end:
    end
.close

.create "build/a012/2_420", 0
map_script 2, 7
.byte 0
.align 4
.close
