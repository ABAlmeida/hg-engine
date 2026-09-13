.nds

.include "build/armips_config.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "build/generated/armips_items.s"
.include "build/generated/armips_species.s"

ELM_ASSISTANT_TABLE_ENTRY equ 0x0C
ELM_ASSISTANT_OBJECT_ID   equ 2
PLAYER_OBJECT_ID          equ 255
ELM_EGG_INTRO_MESSAGE     equ 107
ELM_EGG_RECEIVED_MESSAGE  equ 108
ELM_EGG_CARE_MESSAGE      equ 109
ELM_ASSISTANT_HATCH_MESSAGE equ 110
ELM_ASSISTANT_LEVEL_MESSAGE equ 111
ELM_LAPTOP_INTRO_MESSAGE   equ 112
ELM_LAPTOP_USE_MESSAGE     equ 113
ELM_ASSISTANT_PLAYER_RETREAT_MOVEMENT equ 0x21C
ELM_REQUIRED_DEPARTURE_LEVEL equ 5
PARTY_SLOT_NOT_FOUND equ 255

.if IMPLEMENT_REUSABLE_HEALER

// Elm's starter sequence closes message 11 here and normally turns him back
// toward the player. Redirect that movement command to the appended gift
// routine, then resume immediately after the original WaitMovement command.
ELM_GIFT_HOOK_OFFSET        equ 0x2C5
ELM_GIFT_RETURN_OFFSET      equ 0x2CF
ELM_ORIGINAL_WAIT_OFFSET    equ 0x2CD
ELM_TURN_TO_PLAYER_MOVEMENT equ 0x390
ELM_SHOW_HOOK_OFFSET        equ 0x55F
ELM_SHOW_RETURN_OFFSET      equ 0x569
ELMS_LAB_ORIGINAL_END       equ 0x1158
ELM_OBJECT_ID               equ 0

SCRIPT_OPCODE_GOTO                    equ 22
SCRIPT_OPCODE_APPLY_MOVEMENT          equ 94
SCRIPT_OPCODE_WAIT_MOVEMENT           equ 95
SCRIPT_OPCODE_GET_PARTYMON_SPECIES    equ 354
SCRIPT_OPCODE_GET_PARTY_LEAD_ALIVE    equ 529

ELM_ORIGINAL_MOVEMENT_DISTANCE equ ELM_TURN_TO_PLAYER_MOVEMENT - ELM_ORIGINAL_WAIT_OFFSET
ELM_GIFT_BRANCH_DISTANCE        equ ELMS_LAB_ORIGINAL_END - (ELM_GIFT_HOOK_OFFSET + 6)

.macro assert_elm_gift_hook,file
    // Accept the pristine ApplyMovement command or this patch's existing
    // branch so rebuild_scripts remains idempotent.
    .if readu16(file, ELM_GIFT_HOOK_OFFSET) == SCRIPT_OPCODE_APPLY_MOVEMENT
        .if readu16(file, ELM_GIFT_HOOK_OFFSET + 2) != ELM_OBJECT_ID || readu32(file, ELM_GIFT_HOOK_OFFSET + 4) != ELM_ORIGINAL_MOVEMENT_DISTANCE
            .error "Elm Healing Kit patch found an unexpected starter script"
        .endif
    .elseif readu16(file, ELM_GIFT_HOOK_OFFSET) == SCRIPT_OPCODE_GOTO
        .if readu32(file, ELM_GIFT_HOOK_OFFSET + 2) != ELM_GIFT_BRANCH_DISTANCE
            .error "Elm Healing Kit patch found an unexpected existing branch"
        .endif
    .else
        .error "Elm Healing Kit patch found an unexpected hook command"
    .endif
    .if readu16(file, ELM_ORIGINAL_WAIT_OFFSET) != SCRIPT_OPCODE_WAIT_MOVEMENT
        .error "Elm Healing Kit patch found an unexpected return command"
    .endif
.endmacro

assert_elm_gift_hook "build/a012/2_843"

.open "build/a012/2_843", 0

.org ELM_GIFT_HOOK_OFFSET
goto elm_give_healing_kit

// Append the gift rather than overwriting another part of Elm's script. Replay
// and finish the displaced movement first so Elm faces the player throughout
// the gifts, then resume after the original WaitMovement command.
.org ELMS_LAB_ORIGINAL_END
elm_give_healing_kit:
apply_movement ELM_OBJECT_ID, ELM_TURN_TO_PLAYER_MOVEMENT
wait_movement
setvar VAR_SPECIAL_x8004, ITEM_HEALING_KIT
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
.if IMPLEMENT_LAPTOP
npc_msg ELM_LAPTOP_INTRO_MESSAGE
setvar VAR_SPECIAL_x8004, ITEM_LAPTOP
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
npc_msg ELM_LAPTOP_USE_MESSAGE
.endif
.if IMPLEMENT_REVISED_OPENING
closemsg
// The party contains only the new starter here, so the specialized story-Egg
// command cannot fail for lack of space and preserves Elm's hatch tracking.
npc_msg ELM_EGG_INTRO_MESSAGE
give_togepi_egg
buffer_players_name 0
npc_msg ELM_EGG_RECEIVED_MESSAGE
play_fanfare SEQ_ME_TAMAGO_GET
wait_fanfare
npc_msg ELM_EGG_CARE_MESSAGE
wait_button
closemsg
buffer_players_name 0
npc_msg 106
register_gear_number PHONE_CONTACT_PROF__ELM
play_fanfare SEQ_ME_POKEGEAR_REGIST
wait_fanfare
wait_button
.endif
closemsg
goto ELM_GIFT_RETURN_OFFSET

.if IMPLEMENT_REVISED_OPENING
// Elm's original hatch-reward branch only recognizes the tracked Togepi line
// when it is the first usable party member. Redirect that two-command lookup
// to a party-wide search, then resume at the original phone-trigger cleanup.
.macro assert_elm_show_hook,file
    .if readu16(file, ELM_SHOW_HOOK_OFFSET) == SCRIPT_OPCODE_GET_PARTY_LEAD_ALIVE
        .if readu16(file, ELM_SHOW_HOOK_OFFSET + 2) != VAR_TEMP_x4000 || readu16(file, ELM_SHOW_HOOK_OFFSET + 4) != SCRIPT_OPCODE_GET_PARTYMON_SPECIES || readu16(file, ELM_SHOW_HOOK_OFFSET + 6) != VAR_TEMP_x4000 || readu16(file, ELM_SHOW_HOOK_OFFSET + 8) != VAR_TEMP_x4001
            .error "Revised opening found an unexpected Elm hatch-show lookup"
        .endif
    .elseif readu16(file, ELM_SHOW_HOOK_OFFSET) == SCRIPT_OPCODE_GOTO
        .if readu32(file, ELM_SHOW_HOOK_OFFSET + 2) != (elm_find_hatched_mon_for_show - (ELM_SHOW_HOOK_OFFSET + 6))
            .error "Revised opening found an unexpected existing Elm hatch-show branch"
        .endif
    .else
        .error "Revised opening found an unexpected Elm hatch-show command"
    .endif
.endmacro

assert_elm_show_hook "build/a012/2_843"

.org ELM_SHOW_HOOK_OFFSET
goto elm_find_hatched_mon_for_show

// Script-table entry 3 is the assistant who stops the player at the lab exit.
// Require the tracked Egg's completed Elm interaction and level 5 before
// checking all four gifts. This keeps both the gate and the gift retryable.
.if readu32("build/a012/2_843", ELM_ASSISTANT_TABLE_ENTRY) != 0x654 && readu32("build/a012/2_843", ELM_ASSISTANT_TABLE_ENTRY) != (elm_assistant_opening_supplies - 0x10)
    .error "Revised opening found an unexpected Elm assistant script"
.endif
.org ELM_ASSISTANT_TABLE_ENTRY
.word elm_assistant_opening_supplies - 0x10

.org 0x11C0
elm_assistant_opening_supplies:
scrcmd_609
lockall
get_player_coords VAR_TEMP_x4000, VAR_TEMP_x4001
compare VAR_TEMP_x4000, 3
goto_if_ne elm_assistant_x4
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x81C
goto elm_assistant_arrived
elm_assistant_x4:
compare VAR_TEMP_x4000, 4
goto_if_ne elm_assistant_x5
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x82C
goto elm_assistant_arrived
elm_assistant_x5:
compare VAR_TEMP_x4000, 5
goto_if_ne elm_assistant_x6
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x83C
goto elm_assistant_arrived
elm_assistant_x6:
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x84C
elm_assistant_arrived:
wait_movement
goto_if_unset FLAG_SHOWED_TOGEPI_TO_ELM, elm_assistant_needs_hatch
call find_elm_egg_party_mon
compare VAR_TEMP_x4002, PARTY_SLOT_NOT_FOUND
goto_if_eq elm_assistant_needs_hatch
mon_get_level VAR_TEMP_x4003, VAR_TEMP_x4002
compare VAR_TEMP_x4003, ELM_REQUIRED_DEPARTURE_LEVEL
goto_if_lt elm_assistant_needs_level
buffer_players_name 0
gender_msgbox 19, 20
goto_if_no_item_space ITEM_POTION, 5, elm_assistant_bag_full
goto_if_no_item_space ITEM_POKE_BALL, 100, elm_assistant_bag_full
goto_if_no_item_space ITEM_POKE_BAIT, 100, elm_assistant_bag_full
goto_if_no_item_space ITEM_OLD_ROD, 1, elm_assistant_bag_full
setvar VAR_SPECIAL_x8004, ITEM_POTION
setvar VAR_SPECIAL_x8005, 5
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_POKE_BALL
setvar VAR_SPECIAL_x8005, 100
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_POKE_BAIT
setvar VAR_SPECIAL_x8005, 100
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setflag FLAG_GOT_OLD_ROD
closemsg
// Hide the New Bark counterpart before the exterior map loads. The revised
// New Bark setup preserves these flags across the post-battle field reload.
setflag FLAG_HIDE_NEW_BARK_FRIEND
setflag FLAG_HIDE_NEW_BARK_MARILL
setvar VAR_SCENE_ELMS_LAB, 2
setvar VAR_UNK_408B, 1
clearflag FLAG_HIDE_ROUTE_29_FRIEND
clearflag FLAG_HIDE_ROUTE_29_MARILL
npc_msg 21
wait_button
closemsg
elm_assistant_begin_departure:
compare VAR_TEMP_x4000, 3
goto_if_ne elm_assistant_depart_x4
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x864
goto elm_assistant_depart
elm_assistant_depart_x4:
compare VAR_TEMP_x4000, 4
goto_if_ne elm_assistant_depart_x5
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x870
goto elm_assistant_depart
elm_assistant_depart_x5:
compare VAR_TEMP_x4000, 5
goto_if_ne elm_assistant_depart_x6
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x87C
goto elm_assistant_depart
elm_assistant_depart_x6:
apply_movement ELM_ASSISTANT_OBJECT_ID, 0x888
elm_assistant_depart:
wait_movement
releaseall
end
elm_assistant_bag_full:
callstd std_bag_is_full
closemsg
releaseall
end

elm_assistant_needs_hatch:
npc_msg ELM_ASSISTANT_HATCH_MESSAGE
goto elm_assistant_turn_player_back

elm_assistant_needs_level:
npc_msg ELM_ASSISTANT_LEVEL_MESSAGE

elm_assistant_turn_player_back:
wait_button
closemsg
// Reuse the lab's original one-step retreat movement so the coordinate event
// can trigger again instead of releasing the player beyond the exit gate.
apply_movement PLAYER_OBJECT_ID, ELM_ASSISTANT_PLAYER_RETREAT_MOVEMENT
goto elm_assistant_begin_departure

// Preserve Elm's original VAR_TEMP_x4000/x4001 outputs while accepting the
// tracked Egg's evolution line anywhere in the party.
elm_find_hatched_mon_for_show:
call find_elm_egg_party_mon
copyvar VAR_TEMP_x4000, VAR_TEMP_x4002
copyvar VAR_TEMP_x4001, VAR_TEMP_x4003
goto ELM_SHOW_RETURN_OFFSET

// Returns the matching party slot in x4002 and its species in x4003. A slot
// value of PARTY_SLOT_NOT_FOUND and SPECIES_NONE mean that no accepted
// species was found.
find_elm_egg_party_mon:
get_party_slot_with_species VAR_TEMP_x4002, SPECIES_TOGEPI
compare VAR_TEMP_x4002, PARTY_SLOT_NOT_FOUND
goto_if_ne elm_egg_found_togepi
get_party_slot_with_species VAR_TEMP_x4002, SPECIES_TOGETIC
compare VAR_TEMP_x4002, PARTY_SLOT_NOT_FOUND
goto_if_ne elm_egg_found_togetic
get_party_slot_with_species VAR_TEMP_x4002, SPECIES_TOGEKISS
compare VAR_TEMP_x4002, PARTY_SLOT_NOT_FOUND
goto_if_ne elm_egg_found_togekiss
setvar VAR_TEMP_x4003, SPECIES_NONE
return

elm_egg_found_togepi:
setvar VAR_TEMP_x4003, SPECIES_TOGEPI
return

elm_egg_found_togetic:
setvar VAR_TEMP_x4003, SPECIES_TOGETIC
return

elm_egg_found_togekiss:
setvar VAR_TEMP_x4003, SPECIES_TOGEKISS
return
.endif

.close

.endif
