.nds
.thumb

// Included by armips/global.s after config.s and the generated item constants.

.if IMPLEMENT_BAIT_ENCOUNTERS

.macro assert_thumb_bl,file,offset
    .if (readu16(file, offset) & 0xF800) != 0xF000 || (readu16(file, offset + 2) & 0xF800) != 0xF800
        .error "Bait passive-encounter patch no longer points at the expected Thumb BL instruction"
    .endif
.endmacro

.macro assert_item_patch,file,offset,original,replacement
    .if readu16(file, offset) != original && readu16(file, offset) != replacement
        .error "Bait acquisition patch found an unexpected item ID"
    .endif
.endmacro

// Ordinary passive land encounters call two generation paths here. Return
// FALSE from those paths while leaving roaming, Safari Zone, Bug-Catching
// Contest, fishing, Rock Smash, Headbutt, and scripted encounters untouched.
assert_thumb_bl "base/overlay/overlay_0002.bin", 0x131A
assert_thumb_bl "base/overlay/overlay_0002.bin", 0x1344

.open "base/overlay/overlay_0002.bin", 0x02245B80

.org 0x02246E9A
.area 4
mov r0, #0
nop
.endarea

.org 0x02246EC4
.area 4
mov r0, #0
nop
.endarea

.close

// Mom's compiled gift table: Repel becomes Poké Bait.
assert_item_patch "base/arm9.bin", 0x10837A, ITEM_REPEL, ITEM_POKE_BAIT

.open "base/arm9.bin", 0x02000000

.org 0x0210837A
.area 2
.halfword ITEM_POKE_BAIT
.endarea

.close

// Pickup's compiled item table: Repel becomes Poké Bait. Rare Candy becomes
// PP Up so the repeatable ability cannot produce repeatable Shiny Bait.
assert_item_patch "base/overlay/overlay_0012.bin", 0x34B4C, ITEM_REPEL, ITEM_POKE_BAIT
assert_item_patch "base/overlay/overlay_0012.bin", 0x34B58, ITEM_RARE_CANDY, ITEM_PP_UP

.open "base/overlay/overlay_0012.bin", 0x022378C0

.org 0x0226C40C
.area 2
.halfword ITEM_POKE_BAIT
.endarea

.org 0x0226C418
.area 2
.halfword ITEM_PP_UP
.endarea

.close

.endif
