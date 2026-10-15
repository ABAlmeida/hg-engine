.nds
.thumb

// US HeartGold overlay 51, TrainerCardMainApp_Init. The card reads all sixteen
// badge flags, but normally clears its expanded badge-layout bit when the
// first Hall of Fame date is absent. Keep that layout enabled from the start
// so the interleaved Gym order can display earned Kanto badges before the League.
// This changes only the UI work flags, not the Champion flag or Hall of Fame date.
// Reference: pret/pokeheartgold asm/overlay_trainer_card_main.s, _021E5BB4;
// main.lsf places this overlay after trainer_card, at 0x021E5AC0.
// scripts/make.py decompresses overlay 51 before this patch is applied.
.if readu16("base/overlay/overlay_0051.bin", 0xF8) != 0x4382 && readu16("base/overlay/overlay_0051.bin", 0xF8) != 0x4302
    .error "Trainer-card badge-layout patch found an unexpected flag instruction"
.endif

.open "base/overlay/overlay_0051.bin", 0x021E5AC0

.org 0x021E5BB8
.area 2
orr r2, r0 // r0 = 1: enable the expanded badge layout instead of clearing it
.endarea

.close
