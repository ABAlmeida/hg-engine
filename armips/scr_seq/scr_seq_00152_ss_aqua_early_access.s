.nds

.include "armips/include/flags.s"

// The Olivine port sailor's first condition originally required the game-clear
// flag. Use the one-time Elm ticket scene instead; the existing item check and
// first-voyage setup remain intact.
.if readu16("build/a012/2_152", 0x140) != 32
    .error "Unexpected S.S. Aqua admission flag-check opcode"
.endif
.if readu16("build/a012/2_152", 0x142) != FLAG_GAME_CLEAR
    .error "Unexpected S.S. Aqua admission flag"
.endif

.open "build/a012/2_152", 0
.org 0x142
.halfword FLAG_GOT_SS_TICKET_FROM_ELM
.close
