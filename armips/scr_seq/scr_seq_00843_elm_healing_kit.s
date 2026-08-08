.nds

.include "build/armips_config.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "build/generated/armips_items.s"

ELM_ASSISTANT_TABLE_ENTRY equ 0x0C
ELM_ASSISTANT_OBJECT_ID   equ 2

.if IMPLEMENT_REUSABLE_HEALER

// Elm's starter sequence closes message 11 here and normally turns him back
// toward the player. Redirect that movement command to the appended gift
// routine, then return to the original WaitMovement command.
ELM_GIFT_HOOK_OFFSET        equ 0x2C5
ELM_GIFT_RETURN_OFFSET      equ 0x2CD
ELM_TURN_TO_PLAYER_MOVEMENT equ 0x390
ELMS_LAB_ORIGINAL_END       equ 0x1158
ELM_OBJECT_ID               equ 0

SCRIPT_OPCODE_GOTO           equ 22
SCRIPT_OPCODE_APPLY_MOVEMENT equ 94
SCRIPT_OPCODE_WAIT_MOVEMENT  equ 95

ELM_ORIGINAL_MOVEMENT_DISTANCE equ ELM_TURN_TO_PLAYER_MOVEMENT - ELM_GIFT_RETURN_OFFSET
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
    .if readu16(file, ELM_GIFT_RETURN_OFFSET) != SCRIPT_OPCODE_WAIT_MOVEMENT
        .error "Elm Healing Kit patch found an unexpected return command"
    .endif
.endmacro

assert_elm_gift_hook "build/a012/2_843"

.open "build/a012/2_843", 0

.org ELM_GIFT_HOOK_OFFSET
goto elm_give_healing_kit

// Append the gift rather than overwriting another part of Elm's script. The
// routine replays the displaced movement before returning to the untouched
// WaitMovement command.
.org ELMS_LAB_ORIGINAL_END
elm_give_healing_kit:
setvar VAR_SPECIAL_x8004, ITEM_HEALING_KIT
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
.if IMPLEMENT_REVISED_OPENING
closemsg
buffer_players_name 0
npc_msg 106
register_gear_number PHONE_CONTACT_PROF__ELM
play_fanfare SEQ_ME_POKEGEAR_REGIST
wait_fanfare
wait_button
.endif
closemsg
apply_movement ELM_OBJECT_ID, ELM_TURN_TO_PLAYER_MOVEMENT
goto ELM_GIFT_RETURN_OFFSET

.if IMPLEMENT_REVISED_OPENING
// Script-table entry 3 is the assistant who stops the player at the lab exit.
// Check all three stacks before granting any so a full Bag cannot produce a
// partial gift that is duplicated when the event is retried.
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
buffer_players_name 0
gender_msgbox 19, 20
goto_if_no_item_space ITEM_POTION, 5, elm_assistant_bag_full
goto_if_no_item_space ITEM_POKE_BALL, 20, elm_assistant_bag_full
goto_if_no_item_space ITEM_POKE_BAIT, 20, elm_assistant_bag_full
setvar VAR_SPECIAL_x8004, ITEM_POTION
setvar VAR_SPECIAL_x8005, 5
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_POKE_BALL
setvar VAR_SPECIAL_x8005, 20
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_POKE_BAIT
setvar VAR_SPECIAL_x8005, 20
callstd std_obtain_item_verbose
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
closemsg
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
.endif

.close

.endif
