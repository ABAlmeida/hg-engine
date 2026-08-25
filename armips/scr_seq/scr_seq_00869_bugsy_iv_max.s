.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "build/armips_config.s"
.include "build/generated/armips_items.s"

SCRIPT_OPCODE_GOTO   equ 22
SCRIPT_OPCODE_SETVAR equ 41

BUGSY_REWARD_OFFSET      equ 0x136
BUGSY_REWARD_NEXT_OFFSET equ (BUGSY_REWARD_OFFSET + 6)
BUGSY_APPEND_OFFSET      equ 0x1D8
BUGSY_TM_EXPLANATION_MSG equ 5
BUGSY_DEMO_COMPLETE_MSG  equ 4

// Bugsy uses a custom Gym Leader script. Replace his retryable TM89 handoff
// with an atomic TM89 + IV Max bundle, reusing the original completion flag.
.if readu16("build/a012/2_869", BUGSY_REWARD_OFFSET) == SCRIPT_OPCODE_SETVAR
    .if readu16("build/a012/2_869", BUGSY_REWARD_OFFSET + 2) != VAR_SPECIAL_x8004 || readu16("build/a012/2_869", BUGSY_REWARD_OFFSET + 4) != ITEM_TM089
        .error "Bugsy reward patch found an unexpected original item-space check"
    .endif
.elseif readu16("build/a012/2_869", BUGSY_REWARD_OFFSET) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_869", BUGSY_REWARD_OFFSET + 2) != (bugsy_reward_bundle - BUGSY_REWARD_NEXT_OFFSET)
        .error "Bugsy reward patch found an unexpected existing redirect"
    .endif
.else
    .error "Bugsy reward patch found an unexpected reward routine"
.endif

.open "build/a012/2_869", 0

.org BUGSY_REWARD_OFFSET
goto bugsy_reward_bundle

.org BUGSY_APPEND_OFFSET
bugsy_reward_bundle:
goto_if_no_item_space ITEM_TM089, 1, bugsy_reward_bag_full
goto_if_no_item_space ITEM_IV_MAX, 1, bugsy_reward_bag_full
giveitem_no_check ITEM_TM089, 1
giveitem_no_check ITEM_IV_MAX, 1
setflag FLAG_GOT_TM89_FROM_BUGSY
npc_msg BUGSY_TM_EXPLANATION_MSG
wait_button
closemsg
.if HEARTLESS_GOLD_DEMO
bugsy_demo_complete:
npc_msg BUGSY_DEMO_COMPLETE_MSG
wait_button
closemsg
ResetGame
.endif
releaseall
end

bugsy_reward_bag_full:
callstd std_bag_is_full
closemsg
.if HEARTLESS_GOLD_DEMO
goto bugsy_demo_complete
.endif
releaseall
end

.close
