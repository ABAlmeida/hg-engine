.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_WEST equ 14

// R15R0101: stop eastbound travel from Fuchsia, permit return from Route 15.
.create "build/a012/2_204", 0
scrdef fuchsia_route_15_guard
scrdef fuchsia_route_15_intercept
scrdef fuchsia_route_15_init
scrdef_end

fuchsia_route_15_guard:
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq fuchsia_route_15_guard_open
    simple_npc_msg 1
    end
fuchsia_route_15_guard_open:
    simple_npc_msg 0
    end

fuchsia_route_15_intercept:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_EAST
    goto_if_ne fuchsia_route_15_end
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq fuchsia_route_15_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 1
    wait_button
    closemsg
    apply_movement obj_player, fuchsia_route_15_return_west
    wait_movement
    releaseall
fuchsia_route_15_end:
    end

fuchsia_route_15_init:
    check_badge BADGE_SOUL, VAR_TEMP_x400F
    end
.align 4
fuchsia_route_15_return_west:
    step MOVE_WALK_WEST, 1
    step_end
.close

.create "build/a012/2_449", 0
map_script 2, 3
.byte 0
.align 4
.close
