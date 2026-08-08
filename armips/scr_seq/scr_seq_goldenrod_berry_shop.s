.nds

// The Goldenrod Underground merchant still uses the standard special-mart
// flow. Do not enable its vanilla bitter-medicine dialogue branch now that
// the inventory contains Berries.
.if readu16("build/a012/2_094", 0x1A1) != 30 && readu16("build/a012/2_094", 0x1A1) != 31
    .error "Goldenrod Berry Shop flag command has an unexpected opcode"
.endif

.open "build/a012/2_094", 0
.org 0x1A1
.halfword 31 // ClearFlag
.close
