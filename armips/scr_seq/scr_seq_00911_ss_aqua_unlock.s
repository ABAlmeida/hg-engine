.nds

.include "armips/include/flags.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "build/generated/armips_items.s"

OLIVINE_SCRIPT_END equ 0x63C
OLIVINE_ELM_OBJECT equ 9
OLIVINE_ELM_MAPTEMP_FLAG equ FLAG_MAPTEMP_03F

MOVE_FACE_EAST equ 3
MOVE_WALK_WEST equ 14
MOVE_WALK_EAST equ 15
MOVE_WALK_NORTH equ 12

// Olivine City (script member 911). Entry 0 was an inert interaction on the
// rival object; redirect it to the new one-time Elm scene. Entry 12 is the
// existing transition initializer, which is extended to manage Elm's
// temporary visibility and immediate on-frame trigger.
.if filesize("build/a012/2_911") != OLIVINE_SCRIPT_END
    .error "Unexpected Olivine City script size"
.endif
.if readu32("build/a012/2_911", 0x00) != 0xF0
    .error "Unexpected Olivine City entry 0 pointer"
.endif
.if readu32("build/a012/2_911", 0x30) != 0x6D
    .error "Unexpected Olivine City transition entry pointer"
.endif
.if readu16("build/a012/2_911", 0xF4) != 2
    .error "Unexpected inert Olivine City entry 0"
.endif

.open "build/a012/2_911", 0
.org 0x00
.word olivine_elm_ticket_scene - 0x04
.org 0x30
.word olivine_transition_init - 0x34
.org OLIVINE_SCRIPT_END

olivine_elm_ticket_scene:
    // Invalidate the map-local on-frame condition before the first yielding
    // command. It cannot remain armed in a save or across a map transition.
    setvar VAR_TEMP_x400F, 0
    // Be safe against a stale coordinate record or a second queued on-frame
    // invocation: never lock the field or move Elm after the gift is complete.
    goto_if_set FLAG_GOT_SS_TICKET_FROM_ELM, olivine_elm_ticket_scene_end
    scrcmd_609
    lockall
    apply_movement OLIVINE_ELM_OBJECT, olivine_elm_approach
    apply_movement obj_player, olivine_player_face_elm
    wait_movement
    buffer_players_name 0
    npc_msg 9
    wait_button
    closemsg
    npc_msg 10
    wait_button
    closemsg
    npc_msg 11
    closemsg
    // This vanilla flag both unlocks the port and suppresses Elm's original
    // postgame ticket handout in his laboratory.
    setflag FLAG_GOT_SS_TICKET_FROM_ELM
    giveitem_no_check ITEM_SS_TICKET, 1
    npc_msg 12
    wait_button
    closemsg
    apply_movement OLIVINE_ELM_OBJECT, olivine_elm_depart
    wait_movement
    hide_person OLIVINE_ELM_OBJECT
    setflag OLIVINE_ELM_MAPTEMP_FLAG
    releaseall

olivine_elm_ticket_scene_end:
    end

olivine_transition_init:
    // Object flags hide their object while set. Temp state is recalculated on
    // every map load, so Elm exists only between earning the Mineral Badge and
    // completing this scene.
    setflag OLIVINE_ELM_MAPTEMP_FLAG
    setvar VAR_TEMP_x400F, 0
    // Type-2 map-load scripts run without a TaskManager, so 0x8000 special
    // variables are unavailable here. Store the result in ordinary map state.
    check_badge BADGE_MINERAL, VAR_TEMP_x400E
    compare VAR_TEMP_x400E, 1
    goto_if_ne olivine_transition_original
    goto_if_set FLAG_GOT_SS_TICKET_FROM_ELM, olivine_transition_original
    clearflag OLIVINE_ELM_MAPTEMP_FLAG
    setvar VAR_TEMP_x400F, 1
    goto olivine_transition_original

olivine_transition_original:
    // Preserve the original daily Cameron visibility initializer.
    goto_if_unset FLAG_UNK_189, olivine_transition_check_day
    clearflag FLAG_UNK_189
    end

olivine_transition_check_day:
    get_weekday VAR_TEMP_x4000
    compare VAR_TEMP_x4000, 1
    goto_if_eq olivine_transition_show_cameron
    compare VAR_TEMP_x4000, 2
    goto_if_eq olivine_transition_show_cameron
    setflag FLAG_HIDE_CAMERON
    end

olivine_transition_show_cameron:
    clearflag FLAG_HIDE_CAMERON
    end

.align 4
olivine_elm_approach:
    step MOVE_WALK_WEST, 2
    step_end

.align 4
olivine_player_face_elm:
    step MOVE_FACE_EAST, 1
    step_end

.align 4
olivine_elm_depart:
    // Follow the existing rival's verified exit corridor: east to x=277,
    // then north along the clear route until Elm is safely off camera.
    step MOVE_WALK_EAST, 7
    step MOVE_WALK_NORTH, 10
    step_end

.close
