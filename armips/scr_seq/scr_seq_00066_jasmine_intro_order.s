.nds

.include "armips/include/flags.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"

JASMINE_CONDITION_BLOCK_OFFSET equ 0x37
JASMINE_INTRO_ROUTINE_OFFSET equ 0x62
JASMINE_MEDICINE_ROUTINE_OFFSET equ 0xB5

// Olivine Lighthouse light room (script member 66). The original ordering
// checked for the SecretPotion before checking Jasmine's scene state, which
// skipped her introduction when the potion was acquired early. Reorder the
// same-size condition block so the first conversation always establishes the
// sick-Ampharos scene; a later conversation may then offer the medicine.
.if readu16("build/a012/2_066", JASMINE_CONDITION_BLOCK_OFFSET) != 32
    .error "Unexpected Jasmine SecretPotion flag-check opcode"
.endif
.if readu16("build/a012/2_066", JASMINE_CONDITION_BLOCK_OFFSET + 2) != FLAG_GOT_SECRETPOTION
    .error "Unexpected Jasmine SecretPotion flag"
.endif
.if readu16("build/a012/2_066", JASMINE_CONDITION_BLOCK_OFFSET + 11) != 17
    .error "Unexpected Jasmine scene comparison opcode"
.endif
.if readu16("build/a012/2_066", JASMINE_CONDITION_BLOCK_OFFSET + 13) != VAR_SCENE_LIGHTHOUSE_JASMINE
    .error "Unexpected Jasmine scene variable"
.endif
.if readu16("build/a012/2_066", JASMINE_CONDITION_BLOCK_OFFSET + 15) != 0
    .error "Unexpected Jasmine introductory scene value"
.endif

.open "build/a012/2_066", 0
.org JASMINE_CONDITION_BLOCK_OFFSET
compare VAR_SCENE_LIGHTHOUSE_JASMINE, 0
goto_if_eq JASMINE_INTRO_ROUTINE_OFFSET
goto_if_set FLAG_GOT_SECRETPOTION, JASMINE_MEDICINE_ROUTINE_OFFSET
.close
