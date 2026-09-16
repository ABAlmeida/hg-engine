.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"

MOVE_WALK_WEST equ 14

// Ecruteak/Route 42 gatehouse (R42R0101, script member 253).
// BADGE_MINERAL is a temporary stand-in for the future Magnet Train return
// story flag. Keep the unlock check localized here so it can be swapped later.
.create "build/a012/2_253", 0

scrdef route_42_gatehouse_intercept
scrdef route_42_gatehouse_guard
scrdef route_42_gatehouse_init
scrdef_end

route_42_gatehouse_intercept:
    // The coordinate event is shared by both directions. Only stop players
    // heading east toward Route 42; westbound travel must remain safe.
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_EAST
    goto_if_ne route_42_gatehouse_intercept_end
    check_badge BADGE_MINERAL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_42_gatehouse_intercept_end
    play_se SEQ_SE_DP_SELECT
    lockall
    npc_msg 1
    wait_button
    closemsg
    apply_movement obj_player, route_42_gatehouse_turn_back
    wait_movement
    releaseall

route_42_gatehouse_intercept_end:
    end

route_42_gatehouse_guard:
    check_badge BADGE_MINERAL, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_42_gatehouse_guard_open
    simple_npc_msg 1
    end

route_42_gatehouse_guard_open:
    simple_npc_msg 0
    end

route_42_gatehouse_init:
    // Coordinate events still interrupt bicycle movement even when their
    // script immediately exits. Disable the tile at dispatch time once the
    // temporary Mineral Badge unlock is active. Type-2 map-load scripts have
    // no TaskManager and therefore cannot access 0x8000 special variables.
    check_badge BADGE_MINERAL, VAR_TEMP_x400F

route_42_gatehouse_init_end:
    end

.align 4
route_42_gatehouse_turn_back:
    step MOVE_WALK_WEST, 1
    step_end

.close
