.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/flags.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_NORTH equ 12

// Append entries 8-10 while preserving Lavender's original signs and NPCs.
// The southern NPC becomes the temporary Route 12 guard.
.create "build/lavender_gate_scripts", 0
scrdef lavender_guard
scrdef lavender_intercept
scrdef lavender_init
scrdef_end

lavender_guard:
    simple_npc_msg 8
    end

lavender_intercept:
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_SOUTH
    goto_if_ne lavender_end
    check_badge BADGE_SOUL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq lavender_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 8
    wait_button
    closemsg
    apply_movement obj_player, lavender_return_north
    wait_movement
    releaseall
lavender_end:
    end

lavender_init:
    clearflag FLAG_MAPTEMP_03F
    check_badge BADGE_SOUL, VAR_TEMP_x400F
    compare VAR_TEMP_x400F, 1
    goto_if_ne lavender_init_end
    setflag FLAG_MAPTEMP_03F
lavender_init_end:
    end
.align 4
lavender_return_north:
    step MOVE_WALK_NORTH, 1
    step_end
.close

.create "build/a012/2_540", 0
map_script 2, 11
.byte 0
.align 4
.close
