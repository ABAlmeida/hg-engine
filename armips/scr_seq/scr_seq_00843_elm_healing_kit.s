.nds

.include "build/armips_config.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "asm/include/items.inc"

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
closemsg
apply_movement ELM_OBJECT_ID, ELM_TURN_TO_PLAYER_MOVEMENT
goto ELM_GIFT_RETURN_OFFSET

.close

.endif
