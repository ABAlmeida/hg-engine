.nds
.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "build/generated/armips_items.s"

COPYCAT_ORIGINAL_INTERACTION equ 0x5C
COPYCAT_TRANSFORM equ 0x14D
COPYCAT_RESTORE equ 0x1DB
COPYCAT_FACE_WEST equ 0x26C
COPYCAT_OBJECT equ 0
COPYCAT_SCRIPT_END equ filesize("build/a012/2_841")

// T11R0802: redirect both Copycat forms' shared interaction (entry 0).
// Keep the original quest, reward, gendered sprite initializer, and spin/
// transformation helpers intact. Addresses match the source script labels.
.if readu16("build/a012/2_841", 0x1C) != SCRDEF_END_CONSTANT
    .error "Unexpected Copycat script entry table"
.endif
.if readu32("build/a012/2_841", 0) != COPYCAT_ORIGINAL_INTERACTION - 4
    .error "Unexpected Copycat interaction pointer"
.endif
.if readu16("build/a012/2_841", COPYCAT_TRANSFORM) != 386 || readu16("build/a012/2_841", COPYCAT_RESTORE) != 386
    .error "Unexpected Copycat transformation helpers"
.endif

.open "build/a012/2_841", 0
.org 0
scrdef copycat_marsh_badge_interaction
.org COPYCAT_SCRIPT_END

copycat_marsh_badge_interaction:
    // Preserve completed-quest dialogue, including for existing saves.
    getitemquantity ITEM_PASS, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 0
    goto_if_ne COPYCAT_ORIGINAL_INTERACTION
    check_badge BADGE_MARSH, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_eq COPYCAT_ORIGINAL_INTERACTION

    // This path neither starts the doll quest nor consumes a doll already
    // held by an existing save. The original Power Plant check stays intact.
    play_se SEQ_SE_DP_SELECT
    lockall
    faceplayer
    call COPYCAT_TRANSFORM
    buffer_players_name 0
    npc_msg 18
    wait_button
    closemsg
    call COPYCAT_RESTORE
    npc_msg 19
    wait_button
    closemsg
    apply_movement COPYCAT_OBJECT, COPYCAT_FACE_WEST
    wait_movement
    releaseall
    end
.close
