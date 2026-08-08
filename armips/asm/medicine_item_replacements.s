.nds

// Included by armips/global.s after the generated item constants.

.macro assert_medicine_item_patch,file,offset,original,replacement
    .if readu16(file, offset) != original && readu16(file, offset) != replacement
        .error "Medicine acquisition patch found an unexpected item ID"
    .endif
.endmacro

// Pickup's repeatable low-level Antidote reward becomes a Pecha Berry.
assert_medicine_item_patch "base/overlay/overlay_0012.bin", 0x34B46, ITEM_ANTIDOTE, ITEM_PECHA_BERRY

.open "base/overlay/overlay_0012.bin", 0x022378C0

.org 0x0226C406
.area 2
.halfword ITEM_PECHA_BERRY
.endarea

.close
