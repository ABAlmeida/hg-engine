.nds

.include "armips/include/scriptmacros.s"

CIANWOOD_PHARMACY_FALLBACK_OFFSET equ 0x1E
CIANWOOD_PHARMACY_GIVE_POTION_OFFSET equ 0x51

// Cianwood Pharmacy (script member 881). When the SecretPotion has not yet
// been received, route the former Jasmine-scene fallback to the existing gift
// routine. The acquired-item flag still prevents repeat gifts.
.if readu16("build/a012/2_881", CIANWOOD_PHARMACY_FALLBACK_OFFSET) != 22
    .error "Unexpected Cianwood Pharmacy fallback opcode"
.endif
.if readu32("build/a012/2_881", CIANWOOD_PHARMACY_FALLBACK_OFFSET + 2) != 0
    .error "Unexpected Cianwood Pharmacy fallback destination"
.endif

.open "build/a012/2_881", 0
.org CIANWOOD_PHARMACY_FALLBACK_OFFSET
goto CIANWOOD_PHARMACY_GIVE_POTION_OFFSET
.close
