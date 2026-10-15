.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_SOUTH equ 13
MOVE_WALK_NORTH equ 12
MOVE_WALK_WEST equ 14

// R11R0101: keep the guard's existing entry 1 and the transition's 0x4085
// reset. Entry 0 now intercepts eastbound travel; entry 2 initializes the gate.
.create "build/a012/2_198", 0
scrdef route_11_intercept
scrdef route_11_guard
scrdef route_11_init
scrdef_end

route_11_intercept:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_EAST
    goto_if_ne route_11_end
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_11_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 0
    wait_button
    closemsg
    apply_movement obj_player, route_11_turn_back
    wait_movement
    releaseall
route_11_end:
    end

route_11_guard:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_11_guard_open
    simple_npc_msg 0
    end
route_11_guard_open:
    simple_npc_msg 1
    end

route_11_init:
    setvar VAR_UNK_4085, 0
    // Ordinary map variables are required by synchronous type-2 scripts.
    // Disabling the coordinate event avoids bicycle interruptions after unlock.
    check_badge BADGE_SOUL, VAR_TEMP_x400F
    end

.align 4
route_11_turn_back:
    step MOVE_WALK_WEST, 1
    step_end
.close

// R06R0201 is the Saffron gate, not R06R0101 (the Underground Path entrance).
// Preserve the original Power Plant-dependent conversation after unlock.
.create "build/a012/2_185", 0
scrdef route_6_guard
scrdef route_6_intercept
scrdef route_6_init
scrdef_end

route_6_guard:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_6_guard_open
    simple_npc_msg 2
    end
route_6_guard_open:
    play_se SEQ_SE_DP_SELECT
    lockall
    faceplayer
    goto_if_set FLAG_RESTORED_POWER, route_6_guard_power_restored
    npc_msg 0
    goto route_6_guard_close
route_6_guard_power_restored:
    npc_msg 1
route_6_guard_close:
    wait_button
    closemsg
    releaseall
    end

route_6_intercept:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_6_end
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_NORTH
    goto_if_eq route_6_from_route
    compare VAR_SPECIAL_RESULT, DIR_SOUTH
    goto_if_ne route_6_end
    call route_6_stop_message
    apply_movement obj_player, route_6_return_saffron
    goto route_6_finish_turnback
route_6_from_route:
    call route_6_stop_message
    apply_movement obj_player, route_6_turn_back
route_6_finish_turnback:
    wait_movement
    releaseall
route_6_end:
    end

route_6_stop_message:
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 2
    wait_button
    closemsg
    return

route_6_init:
    check_badge BADGE_SOUL, VAR_TEMP_x400F
    end

.align 4
route_6_turn_back:
    step MOVE_WALK_SOUTH, 1
    step_end
.align 4
route_6_return_saffron:
    step MOVE_WALK_NORTH, 1
    step_end
.close

// Both gates use entry 2 (one-based ID 3) as their transition initializer.
.create "build/a012/2_443", 0
map_script 2, 3
.byte 0
.align 4
.close

.create "build/a012/2_430", 0
map_script 2, 3
.byte 0
.align 4
.close

// Route 11: initialize visibility before map objects are created, using the
// original initializer's entry ID (one-based 2).
.create "build/a012/2_442", 0
map_script 2, 2
.byte 0
.align 4
.close

// Redirect the removed Snorlax's interaction and initializer. Retain both
// sign scripts without changing their relative pointers or message IDs.
ROUTE_11_SCRIPT_END equ filesize("build/a012/2_197")
.if readu16("build/a012/2_197", 0x10) != SCRDEF_END_CONSTANT
    .error "Unexpected Route 11 script entry table"
.endif
.open "build/a012/2_197", 0
.org 0
scrdef diglett_cave_blocker
scrdef diglett_cave_blocker_init
.org ROUTE_11_SCRIPT_END

diglett_cave_blocker:
    simple_npc_msg 4
    end

diglett_cave_blocker_init:
    // Recompute visibility on every load, including existing saves. This
    // map-temporary flag is independent of the former Snorlax encounter flags.
    clearflag FLAG_MAPTEMP_03F
    check_badge BADGE_THUNDER, VAR_TEMP_x400F
    compare VAR_TEMP_x400F, 1
    goto_if_ne diglett_cave_blocker_init_end
    setflag FLAG_MAPTEMP_03F
diglett_cave_blocker_init_end:
    end
.close
