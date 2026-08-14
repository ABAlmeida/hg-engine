.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "build/generated/armips_items.s"

SCRIPT_OPCODE_GOTO   equ 22
SCRIPT_OPCODE_SETVAR equ 41

FALKNER_REWARD_OFFSET       equ 0x126
FALKNER_REWARD_NEXT_OFFSET  equ (FALKNER_REWARD_OFFSET + 6)
FALKNER_APPEND_OFFSET       equ 0x204
FALKNER_TM_EXPLANATION_MSG  equ 4

// Violet Gym uses a custom Gym Leader script, so Falkner never reaches the
// shared trainer-reward table. Redirect his existing retryable TM51 handoff to
// a bundle that checks both pockets before awarding TM51 and IV Max.
.if readu16("build/a012/2_859", FALKNER_REWARD_OFFSET) == SCRIPT_OPCODE_SETVAR
    .if readu16("build/a012/2_859", FALKNER_REWARD_OFFSET + 2) != VAR_SPECIAL_x8004 || readu16("build/a012/2_859", FALKNER_REWARD_OFFSET + 4) != ITEM_TM051
        .error "Falkner reward patch found an unexpected original item-space check"
    .endif
.elseif readu16("build/a012/2_859", FALKNER_REWARD_OFFSET) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_859", FALKNER_REWARD_OFFSET + 2) != (falkner_reward_bundle - FALKNER_REWARD_NEXT_OFFSET)
        .error "Falkner reward patch found an unexpected existing redirect"
    .endif
.else
    .error "Falkner reward patch found an unexpected reward routine"
.endif

.open "build/a012/2_859", 0

.org FALKNER_REWARD_OFFSET
goto falkner_reward_bundle

.org FALKNER_APPEND_OFFSET
falkner_reward_bundle:
goto_if_no_item_space ITEM_TM051, 1, falkner_reward_bag_full
goto_if_no_item_space ITEM_IV_MAX, 1, falkner_reward_bag_full
giveitem_no_check ITEM_TM051, 1
giveitem_no_check ITEM_IV_MAX, 1
setflag FLAG_GOT_TM51_FROM_FALKNER
clearflag FLAG_HIDE_NEW_BARK_FRIENDS_ROOM_FRIEND
// Resume Falkner's original TM explanation and field cleanup after both gifts.
npc_msg FALKNER_TM_EXPLANATION_MSG
wait_button
closemsg
releaseall
end

falkner_reward_bag_full:
callstd std_bag_is_full
closemsg
releaseall
end

.close
