.nds

.include "build/armips_config.s"
.include "armips/include/config.s"
.include "build/generated/armips_items.s"

.if IMPLEMENT_BAIT_ENCOUNTERS

.macro assert_rare_candy_patch,file,offset
    .if readu16(file, offset) != ITEM_RARE_CANDY && readu16(file, offset) != ITEM_SHINY_BAIT
        .error "Shiny Bait reward patch found an unexpected item ID"
    .endif
.endmacro

.macro assert_repel_patch,file,offset
    .if readu16(file, offset) != ITEM_REPEL && readu16(file, offset) != ITEM_SUPER_REPEL && readu16(file, offset) != ITEM_MAX_REPEL && readu16(file, offset) != ITEM_POKE_BAIT
        .error "Poké Bait reward patch found an unexpected item ID"
    .endif
.endmacro

// Fixed NPC Rare Candy awards become Shiny Bait.
// Script member 122: fixed NPC Rare Candy award.
assert_rare_candy_patch "build/a012/2_122", 0x7A1
.open "build/a012/2_122", 0
.org 0x7A1
.halfword ITEM_SHINY_BAIT
.close

// Script member 123: two fixed NPC Rare Candy awards.
assert_rare_candy_patch "build/a012/2_123", 0xC10
assert_rare_candy_patch "build/a012/2_123", 0xCBE
.open "build/a012/2_123", 0
.org 0xC10
.halfword ITEM_SHINY_BAIT
.org 0xCBE
.halfword ITEM_SHINY_BAIT
.close

// Script member 782: fixed NPC Rare Candy award.
assert_rare_candy_patch "build/a012/2_782", 0x63
.open "build/a012/2_782", 0
.org 0x63
.halfword ITEM_SHINY_BAIT
.close

// Item balls: Rare Candy becomes Shiny Bait; all Repel tiers become Poké Bait.
// Script member 141 is the shared item-ball reward table.
assert_rare_candy_patch "build/a012/2_141", 0x46A
assert_repel_patch "build/a012/2_141", 0x64A
assert_rare_candy_patch "build/a012/2_141", 0x9CE
assert_rare_candy_patch "build/a012/2_141", 0xABE
assert_repel_patch "build/a012/2_141", 0xAFA
assert_rare_candy_patch "build/a012/2_141", 0xBFE
assert_rare_candy_patch "build/a012/2_141", 0xD8E
assert_repel_patch "build/a012/2_141", 0xF6E
assert_rare_candy_patch "build/a012/2_141", 0x119E
assert_rare_candy_patch "build/a012/2_141", 0x128E

.open "build/a012/2_141", 0
.org 0x46A
.halfword ITEM_SHINY_BAIT
.org 0x64A
.halfword ITEM_POKE_BAIT
.org 0x9CE
.halfword ITEM_SHINY_BAIT
.org 0xABE
.halfword ITEM_SHINY_BAIT
.org 0xAFA
.halfword ITEM_POKE_BAIT
.org 0xBFE
.halfword ITEM_SHINY_BAIT
.org 0xD8E
.halfword ITEM_SHINY_BAIT
.org 0xF6E
.halfword ITEM_POKE_BAIT
.org 0x119E
.halfword ITEM_SHINY_BAIT
.org 0x128E
.halfword ITEM_SHINY_BAIT
.close

.endif
