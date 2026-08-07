.nds

.include "build/armips_config.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "asm/include/items.inc"
.include "asm/include/species.inc"

SCRIPT_OPCODE_CATCHING_TUTORIAL equ 251
SCRIPT_OPCODE_GOTO              equ 22
SCRIPT_OPCODE_CLOSEMSG          equ 53
SCRIPT_OPCODE_SETVAR            equ 41
SCRIPT_OPCODE_SETFLAG           equ 30
SCRIPT_OPCODE_APPLY_MOVEMENT    equ 94

NEW_BARK_SETUP_TABLE_ENTRY equ 0x24

PLAYER_OBJECT_ID equ 255
MUM_OBJECT_ID    equ 0
SILVER_OBJECT_ID equ 0
NEW_BARK_MARILL_OBJECT_ID equ 3
NEW_BARK_FRIEND_OBJECT_ID equ 4
MR_POKEMON_OAK_OBJECT_ID equ 1
ROUTE_29_MARILL_OBJECT_ID equ 7
ROUTE_29_FRIEND_OBJECT_ID equ 6
VIOLET_ASSISTANT_OBJECT_ID equ 4

TRAINER_SILVER_CHIKORITA equ 495
TRAINER_SILVER_CYNDAQUIL equ 496
TRAINER_SILVER_TOTODILE  equ 497

.if IMPLEMENT_REVISED_OPENING

// Player's house 1F: preserve the automatic opening, then fold Mum's later
// Pokégear, Map, Running Shoes, and savings conversations into it.
.open "build/a012/2_845", 0
.if readu32("build/a012/2_845", 0) != 0x1A && readu32("build/a012/2_845", 0) != (mums_complete_opening - 4)
    .error "Revised opening found an unexpected Mum script table entry"
.endif
.org 0
.word mums_complete_opening - 4

.org 0x490
mums_complete_opening:
scrcmd_609
lockall
apply_movement PLAYER_OBJECT_ID, 0xA4
apply_movement MUM_OBJECT_ID, 0xB0
wait_movement
callstd std_play_mom_music
wait 30, VAR_SPECIAL_RESULT
apply_movement MUM_OBJECT_ID, 0xB8
wait_movement
buffer_players_name 0
gender_msgbox 0, 1
setflag FLAG_GOT_BAG
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 2
setflag FLAG_GOT_TRAINER_CARD
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 3
setflag FLAG_GOT_SAVE_BUTTON
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 4
setflag FLAG_GOT_OPTIONS_BUTTON
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 5

buffer_players_name 0
npc_msg 7
npc_msg 8
setflag FLAG_GOT_POKEGEAR
play_fanfare SEQ_ME_ITEM
wait_fanfare
npc_msg 9
npc_msg 10
touchscreen_menu_hide
getmenuchoice VAR_SPECIAL_RESULT
touchscreen_menu_show
compare VAR_SPECIAL_RESULT, 0
goto_if_ne mum_pokegear_no
npc_msg 11
goto mum_pokegear_done
mum_pokegear_no:
npc_msg 12
mum_pokegear_done:
npc_msg 13

register_pokegear_card 1
play_fanfare SEQ_ME_POKEGEAR_REGIST
wait_fanfare
npc_msg 39
give_running_shoes
play_fanfare SEQ_ME_ITEM
wait_fanfare
npc_msg 40

npc_msg 15
touchscreen_menu_hide
getmenuchoice VAR_SPECIAL_RESULT
touchscreen_menu_show
compare VAR_SPECIAL_RESULT, 0
goto_if_ne mum_savings_no
npc_msg 17
setflag FLAG_SYS_MOMS_SAVINGS
goto mum_savings_done
mum_savings_no:
npc_msg 18
clearflag FLAG_SYS_MOMS_SAVINGS
mum_savings_done:
setflag FLAG_TALKED_TO_MOM_AFTER_NAMING_RIVAL
setvar VAR_SCENE_ROUTE_30_PHONE_CALL, 0
setvar VAR_SCENE_NEW_BARK_WEST_EXIT, 1
setvar VAR_SCENE_CHERRYGROVE_CITY_OW, 2
setflag FLAG_HIDE_CHERRYGROVE_GUIDE_GENT
wait_button
closemsg
wait 15, VAR_SPECIAL_RESULT
apply_movement MUM_OBJECT_ID, 0xC8
wait_movement
callstd std_fade_end_mom_music
setvar VAR_SCENE_PLAYERS_HOUSE_1F, 1
releaseall
end
.close

// New Bark Town: script 8 already runs after the starter sequence. Replace
// its second friend cutscene with the first Silver battle.
.open "build/a012/2_842", 0
.if readu32("build/a012/2_842", 0x20) != 0xEC8 && readu32("build/a012/2_842", 0x20) != (silver_one_after_lab - 0x24)
    .error "Revised opening found an unexpected New Bark script table entry"
.endif
.if readu32("build/a012/2_842", NEW_BARK_SETUP_TABLE_ENTRY) != 0x6D && readu32("build/a012/2_842", NEW_BARK_SETUP_TABLE_ENTRY) != (revised_new_bark_setup - (NEW_BARK_SETUP_TABLE_ENTRY + 4))
    .error "Revised opening found an unexpected New Bark setup script"
.endif
.org 0x20
.word silver_one_after_lab - 0x24
.org NEW_BARK_SETUP_TABLE_ENTRY
.word revised_new_bark_setup - (NEW_BARK_SETUP_TABLE_ENTRY + 4)

.org 0x174C
silver_one_after_lab:
scrcmd_609
lockall
// The original post-starter scene removed these New Bark objects. Preserve
// that cleanup even though Silver now replaces the rest of the scene.
hide_person NEW_BARK_FRIEND_OBJECT_ID
hide_person NEW_BARK_MARILL_OBJECT_ID
setflag FLAG_HIDE_NEW_BARK_FRIEND
setflag FLAG_HIDE_NEW_BARK_MARILL
clearflag FLAG_HIDE_NEW_BARK_RIVAL
// Silver begins at his existing window position. Route him down around the
// lab instead of moving him through its footprint.
apply_movement SILVER_OBJECT_ID, silver_approach
wait_movement
callstd std_play_rival_intro_music
buffer_rivals_name 0
npc_msg 38
closemsg
get_starter_choice VAR_TEMP_x4000
compare VAR_TEMP_x4000, SPECIES_CHIKORITA
goto_if_eq silver_uses_cyndaquil
compare VAR_TEMP_x4000, SPECIES_CYNDAQUIL
goto_if_eq silver_uses_totodile
// Keep the can-lose parameter clear so a loss remains on the black battle
// return screen until the shared white-out task below takes ownership.
trainer_battle TRAINER_SILVER_CHIKORITA, 0, 0, 0
goto silver_battle_done
silver_uses_cyndaquil:
trainer_battle TRAINER_SILVER_CYNDAQUIL, 0, 0, 0
goto silver_battle_done
silver_uses_totodile:
trainer_battle TRAINER_SILVER_TOTODILE, 0, 0, 0
silver_battle_done:
check_battle_won VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_ne silver_won
// Match the shared trainer script so permanent-death wipe handling can run.
white_out
releaseall
end
silver_won:
callstd std_play_rival_outro_music
buffer_rivals_name 0
npc_msg 39
// Silver 1 predates the central trainer-reward system. Award its configured
// rewards here for now; migrate this trainer to trainer_rewards.csv when that
// system is implemented so the player cannot receive them twice.
setvar VAR_SPECIAL_x8004, ITEM_ORAN_BERRY
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setvar VAR_SPECIAL_x8004, ITEM_IV_MAX
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
closemsg
apply_movement SILVER_OBJECT_ID, silver_depart
wait_movement
hide_person SILVER_OBJECT_ID
setflag FLAG_HIDE_NEW_BARK_RIVAL
setflag FLAG_MET_PASSERBY_BOY
setvar VAR_SCENE_NEW_BARK_TOWN_OW, 2
setvar VAR_SCENE_PLAYERS_HOUSE_1F, 4
releaseall
end

.align 4
silver_approach:
step 17, 1 // Leave the window position.
step 18, 1 // Move west around the mailbox at (682, 393).
step 17, 2 // Run south past the mailbox.
step 19, 2 // Run east beside the player.
step 35, 1 // Face east toward the player.
step_end

.align 4
silver_depart:
step 17, 2 // Reach the horizontal path to Route 29.
step 18, 7 // Run west to New Bark's Route 29 exit.
step_end

// Vanilla setup stage 1 explicitly shows the counterpart and Marill for the
// displaced post-starter scene. The revised opening has no such scene, so
// preserve the hide flags already set inside Elm's lab.
.org 0x1840
revised_new_bark_setup:
end
.close

// Route 29: retain the counterpart and Marill grass animation, but replace
// the simulated capture battle and Ball gift with a short Bait explanation.
.open "build/a012/2_225", 0
.if readu16("build/a012/2_225", 0x4FB) == SCRIPT_OPCODE_CATCHING_TUTORIAL
    .if readu16("build/a012/2_225", 0x4FD) != SCRIPT_OPCODE_APPLY_MOVEMENT || readu16("build/a012/2_225", 0x4FF) != ROUTE_29_FRIEND_OBJECT_ID
        .error "Revised opening found an unexpected Route 29 post-tutorial movement"
    .endif
.elseif readu16("build/a012/2_225", 0x4FB) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_225", 0x4FD) != (shortened_catching_tutorial - 0x501)
        .error "Revised opening found an unexpected existing Route 29 branch"
    .endif
.else
    .error "Revised opening found an unexpected Route 29 tutorial command"
.endif
.org 0x4FB
goto shortened_catching_tutorial

.org 0xA14
shortened_catching_tutorial:
gender_msgbox 21, 22
wait_button
closemsg
apply_movement ROUTE_29_FRIEND_OBJECT_ID, 0x928
apply_movement ROUTE_29_MARILL_OBJECT_ID, 0x93C
wait_movement
hide_person ROUTE_29_FRIEND_OBJECT_ID
hide_person ROUTE_29_MARILL_OBJECT_ID
setflag FLAG_HIDE_ROUTE_29_FRIEND
setflag FLAG_HIDE_ROUTE_29_MARILL
setvar VAR_UNK_408B, 0
setflag FLAG_UNK_09A
releaseall
end
.close

// Mr. Pokémon: give the real party Egg at the original Mystery Egg point,
// then continue through Oak's untouched Pokédex sequence.
.open "build/a012/2_229", 0
.if readu16("build/a012/2_229", 0x8B) == SCRIPT_OPCODE_SETVAR
    .if readu16("build/a012/2_229", 0x8D) != VAR_SPECIAL_x8004 || readu16("build/a012/2_229", 0x8F) != ITEM_MYSTERY_EGG
        .error "Revised opening found unexpected Mystery Egg parameters"
    .endif
.elseif readu16("build/a012/2_229", 0x8B) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_229", 0x8D) != (mr_pokemon_gives_real_egg - 0x91)
        .error "Revised opening found an unexpected existing Egg branch"
    .endif
.else
    .error "Revised opening found an unexpected Mystery Egg gift"
.endif
.if (readu16("build/a012/2_229", 0x379) != 1 && readu16("build/a012/2_229", 0x379) != 2) || (readu16("build/a012/2_229", 0x393) != 3 && readu16("build/a012/2_229", 0x393) != 4)
    .error "Revised opening found unexpected Mr. Pokemon scene values"
.endif
.if (readu16("build/a012/2_229", 0x395) != 31 && readu16("build/a012/2_229", 0x395) != SCRIPT_OPCODE_SETFLAG) || (readu16("build/a012/2_229", 0x39D) != 3 && readu16("build/a012/2_229", 0x39D) != 6) || (readu16("build/a012/2_229", 0x39F) != 31 && readu16("build/a012/2_229", 0x39F) != SCRIPT_OPCODE_SETFLAG)
    .error "Revised opening found unexpected rival or Elm return state"
.endif
.if readu16("build/a012/2_229", 0x34F) != SCRIPT_OPCODE_CLOSEMSG && readu16("build/a012/2_229", 0x34F) != SCRIPT_OPCODE_GOTO
    .error "Revised opening found an unexpected Oak departure"
.endif
.org 0x8B
goto mr_pokemon_gives_real_egg

// Give Oak's Fishing Rod before replaying his original departure movement.
// The item keeps the vanilla Old Rod ID for save and engine compatibility.
.org 0x34F
goto oak_gives_old_rod

// Oak's original tail remains in place, but its scene values must describe
// the shortened route instead of enabling the skipped rival/police return.
.org 0x379
.halfword 2 // Mr. Pokémon's house is complete.
.org 0x393
.halfword 4 // Cherrygrove is past both the tour and rival encounter.
.org 0x395
.halfword SCRIPT_OPCODE_SETFLAG // Keep the old Cherrygrove rival hidden.
.org 0x39D
.halfword 6 // Elm's lab is past the police return sequence.
.org 0x39F
.halfword SCRIPT_OPCODE_SETFLAG // Keep the unused lab officer hidden.

.org 0x45C
mr_pokemon_gives_real_egg:
give_togepi_egg
buffer_players_name 0
npc_msg 32
play_fanfare SEQ_ME_TAMAGO_GET
wait_fanfare
npc_msg 3
npc_msg 4
closemsg
fade_screen 6, 1, 0, 0
wait_fade
stop_bgm 0
play_fanfare SEQ_ME_ASA
wait_fanfare
heal_party
fade_screen 6, 1, 1, 0
wait_fade
setflag FLAG_GAVE_RIVAL_NAME_TO_OFFICER
setflag FLAG_GOT_ELMS_PANIC_CALL
setflag FLAG_HIDE_ROUTE_30_BATTLERS
clearflag FLAG_HIDE_ROUTE_30_YOUNGSTER_JOEY
clearflag FLAG_HIDE_CHERRYGROVE_MART_SPECIAL_CLERK
goto 0x2E8

.org 0x4B0
oak_gives_old_rod:
npc_msg 33
setvar VAR_SPECIAL_x8004, ITEM_OLD_ROD
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setflag FLAG_GOT_OLD_ROD
closemsg
apply_movement MR_POKEMON_OAK_OBJECT_ID, 0x450
goto 0x359
.close

// Violet Poké Mart: keep the normal post-Falkner assistant event but turn the
// duplicate Egg handoff into a one-time Shiny Bait reward.
.open "build/a012/2_858", 0
.if readu32("build/a012/2_858", 8) != 0x42 && readu32("build/a012/2_858", 8) != (violet_shiny_bait_reward - 12)
    .error "Revised opening found an unexpected Violet assistant script"
.endif
.org 8
.word violet_shiny_bait_reward - 12

.org 0x184
violet_shiny_bait_reward:
play_se SEQ_SE_DP_SELECT
lockall
faceplayer
buffer_players_name 0
gender_msgbox 2, 3
goto_if_no_item_space ITEM_SHINY_BAIT, 1, violet_bag_full
setvar VAR_SPECIAL_x8004, ITEM_SHINY_BAIT
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
setflag FLAG_GOT_EGG_FROM_ELMS_ASSISTANT
npc_msg 4
wait_button
closemsg
get_player_facing VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 3
goto_if_ne violet_depart_west
apply_movement VIOLET_ASSISTANT_OBJECT_ID, 0x14C
goto violet_depart
violet_depart_west:
apply_movement VIOLET_ASSISTANT_OBJECT_ID, 0x140
violet_depart:
wait_movement
play_se SEQ_SE_DP_KAIDAN2
hide_person VIOLET_ASSISTANT_OBJECT_ID
wait_se SEQ_SE_DP_KAIDAN2
setflag FLAG_HIDE_VIOLET_SHOP_LAB_AIDE
releaseall
setvar VAR_SCENE_VIOLET_CITY_OW, 3
clearflag FLAG_HIDE_VIOLET_KIMONO_GIRL
clearflag FLAG_HIDE_ELMS_LAB_AIDE
setvar VAR_SCENE_ELMS_LAB, 7
end
violet_bag_full:
callstd std_bag_is_full
closemsg
releaseall
end
.close

.endif
