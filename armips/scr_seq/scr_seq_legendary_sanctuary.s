.nds

.include "build/armips_config.s"
.include "armips/include/config.s"
.include "armips/include/flags.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/soundeffects.s"
.include "armips/include/vars.s"
.include "build/generated/armips_items.s"

.if IMPLEMENT_LEGENDARY_SANCTUARY

.macro restart_current_script
    .halfword 21
.endmacro

SANCTUARY_SCRIPT_END         equ 0x848
MAP_SANCTUARY_ROUTE35_GATE   equ 102
SANCTUARY_ROUTE35_ATTENDANT  equ 7
STD_BUG_CONTEST_GUARD_START  equ 10405

MOVE_FACE_NORTH equ 0
MOVE_FACE_SOUTH equ 1
MOVE_WALK_SOUTH equ 13
MOVE_WALK_WEST  equ 14
MOVE_WALK_EAST  equ 15

SANCTUARY_ROUTE35_GATE_SCRIPT_END equ 0x710
SANCTUARY_ROUTE36_GATE_SCRIPT_END equ 0x29C

// Preserve the existing standard-script table and redirect only the Contest
// lifecycle entries. Each expected pointer is from pristine US HeartGold.
.if readu32("build/a012/2_151", 0x00) != 0x98
    .error "Unexpected Bug Contest time-up script pointer"
.endif
.if readu32("build/a012/2_151", 0x04) != 0xAB
    .error "Unexpected Bug Contest balls-out script pointer"
.endif
.if readu32("build/a012/2_151", 0x10) != 0xBA
    .error "Unexpected Bug Contest retire script pointer"
.endif
.if readu32("build/a012/2_151", 0x14) != 0xE5
    .error "Unexpected Bug Contest admission script pointer"
.endif
.if readu32("build/a012/2_151", 0x18) != 0x2EB
    .error "Unexpected Bug Contest exit-guard script pointer"
.endif

.open "build/a012/2_151", 0

.org 0x00
.word sanctuary_time_up - 0x04
.org 0x04
.word sanctuary_balls_out - 0x08
.org 0x10
.word sanctuary_retire_prompt - 0x14
.org 0x14
.word sanctuary_admission - 0x18
.org 0x18
.word sanctuary_exit_prompt - 0x1C
.org SANCTUARY_SCRIPT_END

sanctuary_time_up:
    lockall
    play_se SEQ_SE_DP_PINPON
    npc_msg 0
    wait_button
    closemsg
    call sanctuary_end_session
    call sanctuary_warp_out
    releaseall
    end

sanctuary_balls_out:
    lockall
    play_se SEQ_SE_DP_PINPON
    npc_msg 1
    wait_button
    closemsg
    call sanctuary_end_session
    call sanctuary_warp_out
    releaseall
    end

sanctuary_retire_prompt:
    lockall
    BugContestGetTimeLeft 0
    npc_msg 3
    yesno VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq sanctuary_continue
    closemsg
    call sanctuary_end_session
    call sanctuary_warp_out
    releaseall
    end

sanctuary_continue:
    npc_msg 26
    wait_button
    closemsg
    releaseall
    end

sanctuary_admission:
    compare VAR_UNK_4118, 2
    goto_if_eq sanctuary_post_visit_thanks
    setvar VAR_UNK_4118, 0
.if DEBUG_CHEATS
    // The permanent Pass was removed from the design. In debug builds, its
    // role is represented by the persistent story-unlock flag instead.
    goto_if_set FLAG_SANCTUARY_UNLOCKED, sanctuary_debug_check_permit
    setflag FLAG_SANCTUARY_UNLOCKED

sanctuary_debug_check_permit:
    hasitem ITEM_SANCTUARY_PERMIT, 1, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq sanctuary_debug_items_ready
    giveitem_no_check ITEM_SANCTUARY_PERMIT, 1

sanctuary_debug_items_ready:
.endif
    goto_if_unset FLAG_SANCTUARY_UNLOCKED, sanctuary_locked
.if SANCTUARY_CONTENT_READY == 0
    npc_msg 6
    goto sanctuary_refused
.else
    BugContestAction 2, VAR_SANCTUARY_POOL_STAGE
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne sanctuary_no_eligible
    hasitem ITEM_SANCTUARY_PERMIT, 1, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne sanctuary_no_permit
    npc_msg 4
    yesno VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq sanctuary_declined
    setflag FLAG_BUG_CONTEST_ACTIVE
    setflag FLAG_UNK_24E
    setvar VAR_UNK_4118, 1
    setvar VAR_UNK_40F7, 1
    ScriptOverlayCmd 1, 0
    BugContestAction 0, VAR_SANCTUARY_POOL_STAGE
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne sanctuary_start_failed
    takeitem ITEM_SANCTUARY_PERMIT, 1, VAR_SPECIAL_RESULT
    npc_msg 8
    wait_button
    npc_msg 14
    wait_button
    BugContestGetTimeLeft 0
    npc_msg 15
    wait_button
    setflag 627
    setflag 628
    setflag 629
    setflag 630
    setflag 631
    setflag 632
    setflag 633
    setflag 634
    setflag 635
    setflag 636
    ScriptOverlayCmd 1, 1
    setflag FLAG_UNK_996
    closemsg
    restart_current_script
    end

sanctuary_no_permit:
    npc_msg 5
    goto sanctuary_refused

sanctuary_no_eligible:
    npc_msg 6
    goto sanctuary_refused

sanctuary_declined:
    npc_msg 17
    goto sanctuary_refused

sanctuary_start_failed:
    clearflag FLAG_BUG_CONTEST_ACTIVE
    clearflag FLAG_UNK_24E
    setvar VAR_UNK_40F7, 0
    ScriptOverlayCmd 1, 1
    npc_msg 6
    goto sanctuary_refused
.endif

sanctuary_locked:
    npc_msg 2

sanctuary_admission_done:
    wait_button
    closemsg
    restart_current_script
    end

sanctuary_post_visit_thanks:
    call sanctuary_thank_visitor
    restart_current_script
    end

sanctuary_refused:
    // Both gate callers interpret 1 as permission to warp into the Sanctuary.
    // Any refusal must leave the result at 0 so the player stays in the gate.
    setvar VAR_UNK_4118, 0
    goto sanctuary_admission_done

sanctuary_exit_prompt:
    goto_if_unset FLAG_BUG_CONTEST_ACTIVE, sanctuary_casual_exit
    BugContestGetTimeLeft 0
    npc_msg 23
    yesno VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq sanctuary_continue_from_gate
    closemsg
    call sanctuary_end_session
    call sanctuary_thank_visitor
    restart_current_script
    end

sanctuary_continue_from_gate:
    npc_msg 26
    wait_button
    closemsg
    restart_current_script
    end

sanctuary_casual_exit:
    // A stale or interrupted inactive visit must be allowed to leave. Returning
    // 1 here makes the unmodified gate caller send the player back inside.
    setvar VAR_UNK_4118, 0
    restart_current_script
    end

sanctuary_end_session:
    ScriptOverlayCmd 1, 0
    BugContestAction 1, 0
    ScriptOverlayCmd 1, 1
    fade_out_bgm 0, 30
    reset_bgm
    return

sanctuary_thank_visitor:
    npc_msg 18
    wait_button
    closemsg
    setvar VAR_UNK_4118, 0
    return

sanctuary_warp_out:
    // Script-owned endings present their arrival themselves. Clear the
    // engine-owned arrival marker before changing maps so the gate's on-frame
    // recovery does not try to run the same sequence concurrently.
    setvar VAR_UNK_4118, 0
    fade_screen 6, 1, 0, 0
    wait_fade
    warp MAP_SANCTUARY_ROUTE35_GATE, 0, 25, 2, DIR_SOUTH
    fade_screen 6, 1, 1, 0
    wait_fade
    apply_movement obj_player, sanctuary_route35_arrival_walk
    apply_movement SANCTUARY_ROUTE35_ATTENDANT, sanctuary_route35_attendant_face_player
    wait_movement
    call sanctuary_thank_visitor
    return

.align 4
sanctuary_route35_arrival_walk:
    step MOVE_WALK_SOUTH, 2
    step MOVE_WALK_WEST, 1
    step MOVE_FACE_NORTH, 1
    step_end

.align 4
sanctuary_route35_attendant_face_player:
    step MOVE_FACE_SOUTH, 1
    step_end

.close

// Both gatehouses used weekday checks to swap the Contest attendants. Keep the
// formal-visit attendant visible every day; admission itself is story- and
// Permit-gated.
.if readu32("build/a012/2_242", 0x24) != 0x0E
    .error "Unexpected Route 35 park-gate resume pointer"
.endif
.if readu32("build/a012/2_242", 0x04) != 0x337
    .error "Unexpected Route 35 park-gate attendant pointer"
.endif
.if readu32("build/a012/2_242", 0x30) != 0xBC
    .error "Unexpected Route 35 park-gate init pointer"
.endif
.open "build/a012/2_242", 0
.org 0x04
.word sanctuary_route35_guard_intercept - 0x08
.org 0x24
.word sanctuary_route35_resume - 0x28
.org 0x30
.word sanctuary_route35_init - 0x34

// Continue through the pristine attendant script at its accepted branch. The
// automatic approach wrapper supplies the lock, admission call, and result
// check that normally precede this point.
.org 0x358
sanctuary_route35_admitted:

.org 0x698
sanctuary_route35_init:
    clearflag FLAG_UNK_1C4
    setflag FLAG_UNK_1C3
    setflag 627
    setflag 628
    setflag 629
    setflag 630
    setflag 631
    setflag 632
    setflag 633
    setflag 634
    setflag 635
    setflag 636
    end

.org 0x6CC
sanctuary_route35_resume:
    compare VAR_UNK_4118, 2
    goto_if_ne sanctuary_route35_resume_end
    lockall
    apply_movement obj_player, sanctuary_route35_resume_arrival_walk
    apply_movement SANCTUARY_ROUTE35_ATTENDANT, sanctuary_route35_resume_attendant_face_player
    wait_movement
    callstd STD_BUG_CONTEST_GUARD_START
    releaseall

sanctuary_route35_resume_end:
    end

.align 4
sanctuary_route35_resume_arrival_walk:
    step MOVE_WALK_SOUTH, 2
    step MOVE_WALK_WEST, 1
    step MOVE_FACE_NORTH, 1
    step_end

.align 4
sanctuary_route35_resume_attendant_face_player:
    step MOVE_FACE_SOUTH, 1
    step_end

.org SANCTUARY_ROUTE35_GATE_SCRIPT_END
sanctuary_route35_guard_intercept:
    // Only northward travel is an attempted Sanctuary admission. The same
    // coordinate event is inert when approached from any other direction.
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_NORTH
    goto_if_ne sanctuary_route35_guard_intercept_end
    play_se SEQ_SE_DP_SELECT
    lockall
    callstd STD_BUG_CONTEST_GUARD_START
    compare VAR_UNK_4118, 1
    goto_if_eq sanctuary_route35_admitted
    apply_movement obj_player, sanctuary_route35_turn_away
    wait_movement
    releaseall
    end

sanctuary_route35_guard_intercept_end:
    end

.align 4
sanctuary_route35_turn_away:
    step MOVE_WALK_SOUTH, 1
    step_end
.close

// Extend the existing on-frame table with the engine-owned post-visit state.
// Script 10 is the redirected resume entry above (table index 9 plus one).
.if readu16("build/a012/2_487", 0x16) != 0
    .error "Unexpected Route 35 park-gate on-frame terminator"
.endif
.open "build/a012/2_487", 0
.org 0x16
.halfword VAR_UNK_4118, 2, 10
.halfword 0
.close

.if readu32("build/a012/2_245", 0x00) != 0x0A
    .error "Unexpected Route 36 park-gate init pointer"
.endif
.if readu32("build/a012/2_245", 0x04) != 0x130
    .error "Unexpected Route 36 park-gate attendant pointer"
.endif
.open "build/a012/2_245", 0
.org 0x00
.word sanctuary_route36_init - 0x04
.org 0x04
.word sanctuary_route36_guard_intercept - 0x08

// Continue through the pristine attendant script at its accepted branch,
// matching the Route 35 interception above.
.org 0x151
sanctuary_route36_admitted:

.org 0x268
sanctuary_route36_init:
    clearflag FLAG_UNK_1C4
    setflag FLAG_UNK_1C3
    setflag 627
    setflag 628
    setflag 629
    setflag 630
    setflag 631
    setflag 632
    setflag 633
    setflag 634
    setflag 635
    setflag 636
    end

.org SANCTUARY_ROUTE36_GATE_SCRIPT_END
sanctuary_route36_guard_intercept:
    // Only westward travel is an attempted Sanctuary admission. The same
    // coordinate event is inert when approached from any other direction.
    get_player_facing VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, DIR_WEST
    goto_if_ne sanctuary_route36_guard_intercept_end
    play_se SEQ_SE_DP_SELECT
    lockall
    callstd STD_BUG_CONTEST_GUARD_START
    compare VAR_UNK_4118, 1
    goto_if_eq sanctuary_route36_admitted
    apply_movement obj_player, sanctuary_route36_turn_away
    wait_movement
    releaseall
    end

sanctuary_route36_guard_intercept_end:
    end

.align 4
sanctuary_route36_turn_away:
    step MOVE_WALK_EAST, 1
    step_end
.close

.endif
