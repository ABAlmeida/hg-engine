.nds

.include "build/armips_config.s"
.include "armips/include/config.s"
.include "armips/include/flags.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/soundeffects.s"
.include "armips/include/vars.s"
.include "build/generated/armips_items.s"

.if IMPLEMENT_BAIT_ENCOUNTERS

// Olivine's Good Rod gift already has a persistent completion flag and Bag
// capacity check. Only its awarded item needs to change.
.if readu16("build/a012/2_918", 0x44) != ITEM_GOOD_ROD && readu16("build/a012/2_918", 0x44) != ITEM_SHINY_BAIT
    .error "Olivine Shiny Bait reward found an unexpected item ID"
.endif
.open "build/a012/2_918", 0
.org 0x44
.halfword ITEM_SHINY_BAIT
.close

// Route 12 originally treated possession of the Super Rod as its completion
// state. Shiny Bait is consumable, so use a dedicated persistent variable to
// keep the replacement reward one-time and retryable when the Bag is full.
.create "build/a012/2_200", 0

scrdef route_12_shiny_bait_gift
scrdef_end

route_12_shiny_bait_gift:
    play_se SEQ_SE_DP_SELECT
    lockall
    faceplayer
    compare VAR_GOT_ROUTE_12_SHINY_BAIT, 1
    goto_if_eq route_12_shiny_bait_already_received
    npc_msg 0
    yesno VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq route_12_shiny_bait_declined
    goto_if_no_item_space ITEM_SHINY_BAIT, 1, route_12_shiny_bait_bag_full
    setvar VAR_SPECIAL_x8004, ITEM_SHINY_BAIT
    setvar VAR_SPECIAL_x8005, 1
    callstd std_obtain_item_verbose
    setvar VAR_GOT_ROUTE_12_SHINY_BAIT, 1
    npc_msg 1
    wait_button
    closemsg
    releaseall
    end

route_12_shiny_bait_bag_full:
    callstd std_bag_is_full
    closemsg
    releaseall
    end

route_12_shiny_bait_declined:
    npc_msg 2
    wait_button
    closemsg
    releaseall
    end

route_12_shiny_bait_already_received:
    npc_msg 3
    wait_button
    closemsg
    releaseall
    end

.close

.endif
