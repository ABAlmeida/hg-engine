.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "build/generated/armips_species.s"

UNION_CAVE_LAPRAS_OBJECT_ID equ 3

// Union Cave B2F (script member 58). The original script restricted Lapras
// to Fridays and used a daily flag. This replacement makes the encounter
// permanently available until the first battle finishes, regardless of its
// outcome.
.create "build/a012/2_058", 0

scrdef union_cave_lapras_battle
scrdef union_cave_lapras_hide_during_battle
scrdef union_cave_lapras_update_visibility
scrdef_end

union_cave_lapras_update_visibility:
    goto_if_set FLAG_UNION_CAVE_LAPRAS_CONSUMED, union_cave_lapras_hide
    clearflag FLAG_HIDE_UNION_CAVE_LAPRAS
    end

union_cave_lapras_hide:
    setflag FLAG_HIDE_UNION_CAVE_LAPRAS
    end

union_cave_lapras_hide_during_battle:
    goto_if_set FLAG_ENGAGING_STATIC_POKEMON, union_cave_lapras_hide_active_object
    end

union_cave_lapras_hide_active_object:
    setflag FLAG_HIDE_UNION_CAVE_LAPRAS
    hide_person UNION_CAVE_LAPRAS_OBJECT_ID
    end

union_cave_lapras_battle:
    play_se SEQ_SE_DP_SELECT
    lockall
    faceplayer
    play_cry SPECIES_LAPRAS, 0
    wait_cry
    setflag FLAG_ENGAGING_STATIC_POKEMON
    wild_battle SPECIES_LAPRAS, 20, 0
    clearflag FLAG_ENGAGING_STATIC_POKEMON

    // WildBattle returns for catches, knockouts, fleeing, and losses. Set the
    // persistent flag before checking for blackout so every outcome consumes
    // this encounter.
    setflag FLAG_UNION_CAVE_LAPRAS_CONSUMED
    check_battle_won VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 0
    goto_if_eq union_cave_lapras_blackout
    releaseall
    end

union_cave_lapras_blackout:
    white_out
    releaseall
    end

.close
