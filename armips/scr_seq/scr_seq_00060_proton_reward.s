.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "build/generated/armips_items.s"

SCRIPT_OPCODE_GOTO    equ 22
SCRIPT_OPCODE_FADE_SCREEN equ 174

PROTON_VICTORY_OFFSET      equ 0x80
PROTON_VICTORY_NEXT_OFFSET equ (PROTON_VICTORY_OFFSET + 6)
PROTON_APPEND_OFFSET       equ 0x1FC

// Proton's story battle bypasses the shared trainer-reward script. Redirect
// the won-battle path through an Amulet Coin award, then reproduce the
// overwritten fade command and resume his original story cleanup.
.if readu16("build/a012/2_060", PROTON_VICTORY_OFFSET) == SCRIPT_OPCODE_FADE_SCREEN
.elseif readu16("build/a012/2_060", PROTON_VICTORY_OFFSET) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_060", PROTON_VICTORY_OFFSET + 2) != (proton_reward - PROTON_VICTORY_NEXT_OFFSET)
        .error "Proton reward patch found an unexpected existing redirect"
    .endif
.else
    .error "Proton reward patch found an unexpected victory routine"
.endif

.open "build/a012/2_060", 0

.org PROTON_VICTORY_OFFSET
goto proton_reward

.org PROTON_APPEND_OFFSET
proton_reward:
setvar VAR_SPECIAL_x8004, ITEM_AMULET_COIN
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
fade_screen 6, 1, 0, 0
goto 0x8A

.close
