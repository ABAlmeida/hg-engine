.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_WEST equ 14
MOVE_WALK_EAST equ 15

.create "build/a012/2_207", 0
scrdef route_16_guard
scrdef route_16_intercept
scrdef route_16_require_bike
scrdef route_16_resume
scrdef route_16_init
scrdef_end

route_16_guard:
    check_badge BADGE_RAINBOW, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_16_guard_open
    simple_npc_msg 2
    end
route_16_guard_open:
    simple_npc_msg 0
    end

route_16_intercept:
    check_badge BADGE_RAINBOW, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_16_end
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_WEST
    goto_if_eq route_16_from_east
    compare VAR_SPECIAL_RESULT, DIR_EAST
    goto_if_ne route_16_end
    call route_16_stop_message
    apply_movement obj_player, route_16_return_west
    goto route_16_finish
route_16_from_east:
    call route_16_stop_message
    apply_movement obj_player, route_16_return_east
route_16_finish:
    wait_movement
    releaseall
route_16_end:
    end

route_16_stop_message:
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 2
    wait_button
    closemsg
    return

route_16_require_bike:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_WEST
    goto_if_ne route_16_end
    player_on_bike_check VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_16_now_cycling
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 1
    wait_button
    closemsg
    apply_movement obj_player, route_16_return_east
    goto route_16_finish
route_16_now_cycling:
    setvar VAR_TEMP_x400E, 1
    scrcmd_186 1
    setflag FLAG_UNK_149
    end

route_16_init:
    check_badge BADGE_RAINBOW, VAR_TEMP_x400F
    setvar VAR_TEMP_x400E, 1
    end

route_16_resume:
    // Check avatar state only after the map/avatar has been loaded. Bikers
    // never dispatch a crossing script after unlock, preserving their speed.
    // Resume scripts are synchronous too: use ordinary map variables.
    player_on_bike_check VAR_TEMP_x400E
    compare VAR_TEMP_x400E, 1
    goto_if_eq route_16_resume_cycling
    scrcmd_186 0
    compare VAR_TEMP_x400F, 1
    goto_if_eq route_16_resume_walking_open
    setvar VAR_TEMP_x400E, 1
    end
route_16_resume_walking_open:
    setvar VAR_TEMP_x400E, 0
    end
route_16_resume_cycling:
    setvar VAR_TEMP_x400E, 1
    // Keep the rider mounted while inside the gate. The eastern map clears
    // this lock on arrival; Cycling Road's own initializer retains its lock.
    scrcmd_186 1
    setflag FLAG_UNK_149
    end

.align 4
route_16_return_west:
    step MOVE_WALK_WEST, 1
    step_end
.align 4
route_16_return_east:
    step MOVE_WALK_EAST, 1
    step_end
.close

.create "build/a012/2_452", 0
map_script 2, 5
map_script 3, 4
.byte 0
.align 4
.close

// Preserve the eastern route's original flag clear and explicitly release
// the gatehouse bike lock without a coordinate script during passage.
.create "build/a012/2_208", 0
scrdef route_16_east_init
scrdef_end
route_16_east_init:
    scrcmd_186 0
    clearflag FLAG_UNK_149
    end
.close
