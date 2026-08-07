.nds
.thumb

// Included by armips/global.s after build/armips_config.s.

.if DISABLE_TRAINER_LINE_OF_SIGHT

// US HeartGold calls TryGetSeenByNpcTrainers before processing ordinary field
// input. Returning FALSE here prevents the engine from pausing the field and
// launching std_trainer_approach. Pressing A still launches the trainer's
// object script through the separate interaction path.
.if (readu16("base/arm9.bin", 0x640C8) != 0xB5F8 || readu16("base/arm9.bin", 0x640CA) != 0xB096) && (readu16("base/arm9.bin", 0x640C8) != 0x2000 || readu16("base/arm9.bin", 0x640CA) != 0x4770)
    .error "Trainer line-of-sight patch found an unexpected function prologue"
.endif

.open "base/arm9.bin", 0x02000000

.org 0x020640C8
.area 4
mov r0, #0
bx lr
.endarea

.close

.endif
