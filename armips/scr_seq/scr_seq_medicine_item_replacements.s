.nds

.include "build/generated/armips_items.s"

// Member 141 is the shared visible-item-ball reward table. Preserve each
// item's existing quantity, collection flag, and standard pickup flow by
// replacing only its item ID.
.macro assert_item_ball_patch,offset,original,replacement
    .if readu16("build/a012/2_141", offset) != original && readu16("build/a012/2_141", offset) != replacement
        .error "Medicine item-ball replacement found an unexpected item ID"
    .endif
.endmacro

assert_item_ball_patch 0x41A, ITEM_ANTIDOTE, ITEM_PECHA_BERRY
assert_item_ball_patch 0x492, ITEM_PARALYZE_HEAL, ITEM_CHERI_BERRY
assert_item_ball_patch 0x50A, ITEM_HEAL_POWDER, ITEM_LUM_BERRY
assert_item_ball_patch 0x51E, ITEM_ENERGY_POWDER, ITEM_ORAN_BERRY
assert_item_ball_patch 0x55A, ITEM_HEAL_POWDER, ITEM_LUM_BERRY
assert_item_ball_patch 0x56E, ITEM_ENERGY_ROOT, ITEM_SITRUS_BERRY
assert_item_ball_patch 0x6D6, ITEM_AWAKENING, ITEM_CHESTO_BERRY
assert_item_ball_patch 0x78A, ITEM_ANTIDOTE, ITEM_PECHA_BERRY
assert_item_ball_patch 0x816, ITEM_BURN_HEAL, ITEM_RAWST_BERRY
assert_item_ball_patch 0x8DE, ITEM_PARALYZE_HEAL, ITEM_CHERI_BERRY
assert_item_ball_patch 0x942, ITEM_ANTIDOTE, ITEM_PECHA_BERRY
assert_item_ball_patch 0x155E, ITEM_ICE_HEAL, ITEM_ASPEAR_BERRY

.open "build/a012/2_141", 0
.org 0x41A
.halfword ITEM_PECHA_BERRY
.org 0x492
.halfword ITEM_CHERI_BERRY
.org 0x50A
.halfword ITEM_LUM_BERRY
.org 0x51E
.halfword ITEM_ORAN_BERRY
.org 0x55A
.halfword ITEM_LUM_BERRY
.org 0x56E
.halfword ITEM_SITRUS_BERRY
.org 0x6D6
.halfword ITEM_CHESTO_BERRY
.org 0x78A
.halfword ITEM_PECHA_BERRY
.org 0x816
.halfword ITEM_RAWST_BERRY
.org 0x8DE
.halfword ITEM_CHERI_BERRY
.org 0x942
.halfword ITEM_PECHA_BERRY
.org 0x155E
.halfword ITEM_ASPEAR_BERRY
.close
