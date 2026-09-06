.nds

.include "build/armips_config.s"
.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "armips/include/vars.s"
.include "armips/include/soundeffects.s"
.include "build/generated/armips_items.s"
.include "build/generated/armips_species.s"

SCRIPT_OPCODE_GOTO              equ 22
SCRIPT_OPCODE_GENDER_MSGBOX     equ 132
SCRIPT_OPCODE_PLAY_CRY          equ 76
SCRIPT_OPCODE_CLOSEMSG          equ 53
SCRIPT_OPCODE_SETVAR            equ 41
SCRIPT_OPCODE_SETFLAG           equ 30
SCRIPT_OPCODE_APPLY_MOVEMENT    equ 94
SCRIPT_OPCODE_LOCKALL           equ 96
SCRIPT_OPCODE_SCRCMD_609        equ 609

NEW_BARK_SETUP_TABLE_ENTRY equ 0x24
NEW_BARK_COMMENT_HOOK_OFFSET equ 0xDCC
NEW_BARK_COMMENT_RETURN_OFFSET equ 0xDD4
NEW_BARK_COMMENT_MOVEMENT equ 0xE60
NEW_BARK_LYRA_SLAKOTH_MESSAGE equ 40
NEW_BARK_ETHAN_SLAKOTH_MESSAGE equ 41

ELM_HATCH_REWARD_SETVAR_OFFSET equ 0x5F2
ELM_HATCH_REWARD_ITEM_OFFSET equ ELM_HATCH_REWARD_SETVAR_OFFSET + 4
ELM_HATCH_REWARD_QUANTITY_OFFSET equ ELM_HATCH_REWARD_SETVAR_OFFSET + 6

PLAYER_OBJECT_ID equ 255
MUM_OBJECT_ID    equ 0
SILVER_OBJECT_ID equ 0
NEW_BARK_MARILL_OBJECT_ID equ 3
NEW_BARK_FRIEND_OBJECT_ID equ 4
MR_POKEMON_OAK_OBJECT_ID equ 1
ROUTE_29_MARILL_OBJECT_ID equ 7
ROUTE_29_FRIEND_OBJECT_ID equ 6
VIOLET_ASSISTANT_OBJECT_ID equ 4

ROUTE_29_TUTORIAL_TABLE_ENTRY equ 0x4
ROUTE_29_TUTORIAL_ORIGINAL_OFFSET equ 0x1B2
ROUTE_29_TUTORIAL_APPEND_OFFSET equ 0xA14
ROUTE_29_PLAYER_NORTH_Z equ 396
ROUTE_29_PLAYER_SOUTH_Z equ 402

MOVE_WALK_NORTH equ 12
MOVE_WALK_SOUTH equ 13
MOVE_WALK_WEST equ 14
MOVE_WALK_EAST equ 15
MOVE_RUN_NORTH equ 16
MOVE_RUN_SOUTH equ 17
MOVE_RUN_WEST equ 18
MOVE_RUN_EAST equ 19
MOVE_FACE_EAST equ 35
MOVE_JUMP_ON_SPOT_FAST_WEST equ 50
MOVE_JUMP_FAR_WEST equ 58
MOVE_DELAY_8 equ 63
MOVE_DELAY_16 equ 65
MOVE_EMOTE_EXCLAMATION equ 75

TRAINER_SILVER_CHIKORITA equ 495
TRAINER_SILVER_CYNDAQUIL equ 496
TRAINER_SILVER_TOTODILE  equ 497
TRAINER_LYRA_GATEHOUSE   equ 738
TRAINER_ETHAN_GATEHOUSE  equ 739

.if IMPLEMENT_REVISED_OPENING

// Elm's vanilla post-hatch reward prepares the gift in VAR_SPECIAL_x8004,
// checks Bag space, and gives it through the standard verbose item routine.
// Change only that prepared item so the existing trigger and completion flow
// now award Eviolite instead of Everstone.
.if readu16("build/a012/2_843", ELM_HATCH_REWARD_SETVAR_OFFSET) != SCRIPT_OPCODE_SETVAR || readu16("build/a012/2_843", ELM_HATCH_REWARD_SETVAR_OFFSET + 2) != VAR_SPECIAL_x8004
    .error "Revised opening found an unexpected Elm hatch-reward command"
.endif
.if readu16("build/a012/2_843", ELM_HATCH_REWARD_ITEM_OFFSET) != ITEM_EVERSTONE && readu16("build/a012/2_843", ELM_HATCH_REWARD_ITEM_OFFSET) != ITEM_EVIOLITE
    .error "Revised opening found an unexpected Elm hatch-reward item"
.endif
.if readu16("build/a012/2_843", ELM_HATCH_REWARD_QUANTITY_OFFSET) != SCRIPT_OPCODE_SETVAR || readu16("build/a012/2_843", ELM_HATCH_REWARD_QUANTITY_OFFSET + 2) != VAR_SPECIAL_x8005 || readu16("build/a012/2_843", ELM_HATCH_REWARD_QUANTITY_OFFSET + 4) != 1
    .error "Revised opening found an unexpected Elm hatch-reward quantity"
.endif
.open "build/a012/2_843", 0
.org ELM_HATCH_REWARD_ITEM_OFFSET
.halfword ITEM_EVIOLITE
.close

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
npc_msg 7
npc_msg 8
setflag FLAG_GOT_POKEGEAR
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 9
npc_msg 10
// Preserve Mum's original Pokégear tutorial choice. The message embeds the
// Yes/No menu, but GetMenuChoice is still required to read its result.
touchscreen_menu_hide
getmenuchoice VAR_SPECIAL_RESULT
touchscreen_menu_show
compare VAR_SPECIAL_RESULT, 0
goto_if_ne mum_pokegear_help_no
npc_msg 11
goto mum_pokegear_help_done
mum_pokegear_help_no:
npc_msg 12
mum_pokegear_help_done:
npc_msg 13
wait_button_or_dpad

register_pokegear_card 1
play_fanfare SEQ_ME_POKEGEAR_REGIST
wait_fanfare
npc_msg 39
wait_button
give_running_shoes
play_fanfare SEQ_SE_PL_KIRAKIRA
wait_fanfare
npc_msg 40
wait_button

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

// In the first New Bark companion scene, this is Slakoth's final movement
// before the counterpart leads it away. Insert one line of dialogue here,
// replay the displaced movement, and return to the original WaitMovement.
.if readu16("build/a012/2_842", NEW_BARK_COMMENT_HOOK_OFFSET) == SCRIPT_OPCODE_APPLY_MOVEMENT
    .if readu16("build/a012/2_842", NEW_BARK_COMMENT_HOOK_OFFSET + 2) != NEW_BARK_MARILL_OBJECT_ID || readu32("build/a012/2_842", NEW_BARK_COMMENT_HOOK_OFFSET + 4) != (NEW_BARK_COMMENT_MOVEMENT - NEW_BARK_COMMENT_RETURN_OFFSET)
        .error "Revised opening found an unexpected New Bark companion movement"
    .endif
.elseif readu16("build/a012/2_842", NEW_BARK_COMMENT_HOOK_OFFSET) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_842", NEW_BARK_COMMENT_HOOK_OFFSET + 2) != (counterpart_slakoth_comment - (NEW_BARK_COMMENT_HOOK_OFFSET + 6))
        .error "Revised opening found an unexpected existing companion branch"
    .endif
.else
    .error "Revised opening found an unexpected New Bark companion command"
.endif
.org 0x20
.word silver_one_after_lab - 0x24
.org NEW_BARK_SETUP_TABLE_ENTRY
.word revised_new_bark_setup - (NEW_BARK_SETUP_TABLE_ENTRY + 4)
.org NEW_BARK_COMMENT_HOOK_OFFSET
goto counterpart_slakoth_comment

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
wait_button
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
wait_button
closemsg
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
// End the temporary rival theme and restore New Bark's map music before the
// player regains field control.
callstd std_fade_end_rival_outro_music
releaseall
// The stage-1 New Bark setup is intentionally a no-op. Reuse this existing
// terminator so it cannot overwrite the tightly packed movement lists below.
revised_new_bark_setup:
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
step 17, 2 // South twice.
step 18, 2 // West twice.
step 17, 3 // Reach the horizontal path to Route 29.
step 18, 5 // Run west to New Bark's Route 29 exit.
step_end

counterpart_slakoth_comment:
gender_msgbox NEW_BARK_LYRA_SLAKOTH_MESSAGE, NEW_BARK_ETHAN_SLAKOTH_MESSAGE
wait_button
closemsg
apply_movement NEW_BARK_MARILL_OBJECT_ID, NEW_BARK_COMMENT_MOVEMENT
goto NEW_BARK_COMMENT_RETURN_OFFSET
.close

// Route 29: expose the complete counterpart and Slakoth tutorial in editable
// source. Script-table entry 1 normally targets 0x1B2. Redirect that entry to
// the expanded copy below instead of patching isolated commands in place.
// The new copy retains the original coordinate-dependent staging, gives both
// counterparts the same grass demonstration, and explains Bait without
// starting opcode 251's simulated capture battle or giving more Poké Balls.
.open "build/a012/2_225", 0
.if readu32("build/a012/2_225", ROUTE_29_TUTORIAL_TABLE_ENTRY) != (ROUTE_29_TUTORIAL_ORIGINAL_OFFSET - (ROUTE_29_TUTORIAL_TABLE_ENTRY + 4)) && readu32("build/a012/2_225", ROUTE_29_TUTORIAL_TABLE_ENTRY) != (route_29_catching_tutorial - (ROUTE_29_TUTORIAL_TABLE_ENTRY + 4))
    .error "Revised opening found an unexpected Route 29 tutorial table entry"
.endif
.if readu16("build/a012/2_225", ROUTE_29_TUTORIAL_ORIGINAL_OFFSET) != SCRIPT_OPCODE_SCRCMD_609 || readu16("build/a012/2_225", ROUTE_29_TUTORIAL_ORIGINAL_OFFSET + 2) != SCRIPT_OPCODE_LOCKALL
    .error "Revised opening found an unexpected Route 29 tutorial script"
.endif
.org ROUTE_29_TUTORIAL_TABLE_ENTRY
.word route_29_catching_tutorial - (ROUTE_29_TUTORIAL_TABLE_ENTRY + 4)

.org ROUTE_29_TUTORIAL_APPEND_OFFSET
route_29_catching_tutorial:
scrcmd_609
lockall
play_cry SPECIES_SLAKOTH, 0
wait_cry
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_arrives
wait_movement
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_notices_player
wait_movement
callstd std_play_friend_music
gender_msgbox 17, 18
closemsg
get_player_coords VAR_TEMP_x4000, VAR_TEMP_x4001
release ROUTE_29_MARILL_OBJECT_ID

// Bring the counterpart and Slakoth beside the player from any of the seven
// north/south trigger tiles used by the original Route 29 event.
compare VAR_TEMP_x4001, ROUTE_29_PLAYER_NORTH_Z
goto_if_ne route_29_approach_z397
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z396
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z396
goto route_29_approach_done
route_29_approach_z397:
compare VAR_TEMP_x4001, 397
goto_if_ne route_29_approach_z398
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z397
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z397
goto route_29_approach_done
route_29_approach_z398:
compare VAR_TEMP_x4001, 398
goto_if_ne route_29_approach_z399
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z398
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z398
goto route_29_approach_done
route_29_approach_z399:
compare VAR_TEMP_x4001, 399
goto_if_ne route_29_approach_z400
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z399
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z399
goto route_29_approach_done
route_29_approach_z400:
compare VAR_TEMP_x4001, 400
goto_if_ne route_29_approach_z401
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z400
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z400
goto route_29_approach_done
route_29_approach_z401:
compare VAR_TEMP_x4001, 401
goto_if_ne route_29_approach_z402
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z401
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z401
goto route_29_approach_done
route_29_approach_z402:
compare VAR_TEMP_x4001, ROUTE_29_PLAYER_SOUTH_Z
goto_if_ne route_29_approach_done
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_approach_z402
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_approach_z402
route_29_approach_done:
wait_movement
lock ROUTE_29_MARILL_OBJECT_ID
buffer_players_name 0
gender_msgbox 0, 1
closemsg
get_player_coords VAR_TEMP_x4000, VAR_TEMP_x4001
scrcmd_602 0
wait_following_pokemon_movement
scrcmd_604 55

// Stage all three characters around the grass. Lyra and Ethan use the same
// counterpart choreography; gender changes only their presentation/dialogue.
compare VAR_TEMP_x4001, ROUTE_29_PLAYER_NORTH_Z
goto_if_ne route_29_stage_z397
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z396
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z396
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z396
goto route_29_stage_done
route_29_stage_z397:
compare VAR_TEMP_x4001, 397
goto_if_ne route_29_stage_z398
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z397
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z397
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z397
goto route_29_stage_done
route_29_stage_z398:
compare VAR_TEMP_x4001, 398
goto_if_ne route_29_stage_z399
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z398
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z398
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z398
goto route_29_stage_done
route_29_stage_z399:
compare VAR_TEMP_x4001, 399
goto_if_ne route_29_stage_z400
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z399
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z399
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z399
goto route_29_stage_done
route_29_stage_z400:
compare VAR_TEMP_x4001, 400
goto_if_ne route_29_stage_z401
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z400
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z400
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z400
goto route_29_stage_done
route_29_stage_z401:
compare VAR_TEMP_x4001, 401
goto_if_ne route_29_stage_z402
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z401
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z401
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z401
goto route_29_stage_done
route_29_stage_z402:
compare VAR_TEMP_x4001, ROUTE_29_PLAYER_SOUTH_Z
goto_if_ne route_29_stage_done
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_friend_stage_z402
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_stage_z402
apply_movement PLAYER_OBJECT_ID, route_29_player_stage_z402
route_29_stage_done:
wait_movement
wait_following_pokemon_movement
scrcmd_602 1
scrcmd_604 48

// The gender-selected counterpart performs the same demonstration.
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_counterpart_reacts
wait_movement
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_counterpart_encourages_slakoth
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_jumps_into_grass
wait_movement
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_counterpart_crosses_grass
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_crosses_grass
wait_movement
gender_msgbox 21, 22
wait_button
closemsg
wait 10, VAR_SPECIAL_RESULT
// The grass crossing leaves Slakoth one tile behind the counterpart. Slakoth
// enters the vacated tile, traces the same route, and stops one step behind.
apply_movement ROUTE_29_FRIEND_OBJECT_ID, route_29_counterpart_departs
apply_movement ROUTE_29_MARILL_OBJECT_ID, route_29_slakoth_follows_counterpart
wait_movement
hide_person ROUTE_29_FRIEND_OBJECT_ID
hide_person ROUTE_29_MARILL_OBJECT_ID
setflag FLAG_HIDE_ROUTE_29_FRIEND
setflag FLAG_HIDE_ROUTE_29_MARILL
setvar VAR_UNK_408B, 0
setflag FLAG_UNK_09A
releaseall
end

// Editable movement blocks -------------------------------------------------
// Each step is encoded as `step TYPE, REPEAT_COUNT`. Keep `step_end` at the
// end of every block. Slakoth retains the old MARILL object name only in the
// map data; these labels use its story-facing name.
.align 4
route_29_slakoth_arrives:
step MOVE_RUN_SOUTH, 1
step MOVE_RUN_EAST, 2
step MOVE_RUN_NORTH, 1
step MOVE_JUMP_ON_SPOT_FAST_WEST, 2
step_end

.align 4
route_29_friend_notices_player:
step MOVE_FACE_EAST, 1
step MOVE_EMOTE_EXCLAMATION, 1
step_end

.align 4
route_29_friend_approach_z396:
step MOVE_RUN_NORTH, 4
step MOVE_RUN_EAST, 2
step MOVE_RUN_SOUTH, 2
step MOVE_RUN_WEST, 2
step MOVE_RUN_NORTH, 2
step MOVE_RUN_EAST, 4
step_end

.align 4
route_29_friend_approach_z397:
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 3
step MOVE_WALK_EAST, 2
step_end

.align 4
route_29_friend_approach_z398:
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 2
step MOVE_WALK_EAST, 2
step_end

.align 4
route_29_friend_approach_z399:
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 1
step MOVE_WALK_EAST, 2
step_end

.align 4
route_29_friend_approach_z400:
step MOVE_WALK_EAST, 4
step_end

.align 4
route_29_friend_approach_z401:
step MOVE_WALK_EAST, 2
step MOVE_WALK_SOUTH, 1
step MOVE_WALK_EAST, 2
step_end

.align 4
route_29_friend_approach_z402:
step MOVE_WALK_EAST, 2
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_EAST, 2
step_end

.align 4
route_29_slakoth_approach_z396:
step MOVE_RUN_WEST, 1
step MOVE_RUN_NORTH, 4
step MOVE_RUN_EAST, 2
step MOVE_RUN_SOUTH, 2
step MOVE_RUN_WEST, 2
step MOVE_RUN_NORTH, 2
step MOVE_RUN_EAST, 3
step_end

.align 4
route_29_slakoth_approach_z397:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 3
step MOVE_WALK_EAST, 1
step_end

.align 4
route_29_slakoth_approach_z398:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 2
step MOVE_WALK_EAST, 1
step_end

.align 4
route_29_slakoth_approach_z399:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 2
step MOVE_WALK_NORTH, 1
step MOVE_WALK_EAST, 1
step_end

.align 4
route_29_slakoth_approach_z400:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 3
step_end

.align 4
route_29_slakoth_approach_z401:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 2
step MOVE_WALK_SOUTH, 1
step MOVE_WALK_EAST, 1
step_end

.align 4
route_29_slakoth_approach_z402:
step MOVE_WALK_WEST, 1
step MOVE_WALK_EAST, 2
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_EAST, 1
step_end

.align 4
route_29_friend_stage_z396:
step MOVE_WALK_SOUTH, 6
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z397:
step MOVE_WALK_SOUTH, 5
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z398:
step MOVE_WALK_SOUTH, 4
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z399:
step MOVE_WALK_SOUTH, 3
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z400:
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z401:
step MOVE_WALK_SOUTH, 1
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_friend_stage_z402:
step MOVE_WALK_WEST, 9
step_end

.align 4
route_29_slakoth_stage_z396:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 6
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z397:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 5
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z398:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 4
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z399:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 3
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z400:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z401:
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 1
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_slakoth_stage_z402:
step MOVE_WALK_EAST, 1
step MOVE_WALK_WEST, 8
step_end

.align 4
route_29_player_stage_z396:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 6
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z397:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 5
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z398:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 4
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z399:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 3
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z400:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z401:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 1
step MOVE_WALK_SOUTH, 1
step MOVE_WALK_WEST, 5
step_end

.align 4
route_29_player_stage_z402:
step MOVE_DELAY_8, 1
step MOVE_WALK_WEST, 6
step_end

.align 4
route_29_counterpart_reacts:
step MOVE_EMOTE_EXCLAMATION, 1
step_end

.align 4
route_29_counterpart_encourages_slakoth:
step MOVE_JUMP_ON_SPOT_FAST_WEST, 3
step MOVE_DELAY_16, 1
step MOVE_JUMP_ON_SPOT_FAST_WEST, 3
step MOVE_DELAY_8, 3
step MOVE_JUMP_ON_SPOT_FAST_WEST, 3
step MOVE_DELAY_8, 3
step_end

.align 4
route_29_slakoth_jumps_into_grass:
step MOVE_DELAY_8, 3
step MOVE_JUMP_FAR_WEST, 1
step MOVE_DELAY_8, 3
step MOVE_JUMP_ON_SPOT_FAST_WEST, 3
step MOVE_DELAY_8, 3
step MOVE_JUMP_ON_SPOT_FAST_WEST, 3
step_end

.align 4
route_29_counterpart_crosses_grass:
step MOVE_WALK_EAST, 3
step_end

.align 4
route_29_slakoth_crosses_grass:
step MOVE_WALK_EAST, 3
step_end

.align 4
route_29_counterpart_departs:
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_WEST, 4
step MOVE_WALK_SOUTH, 4
step MOVE_WALK_WEST, 3
step_end

.align 4
route_29_slakoth_follows_counterpart:
// Join the counterpart's starting tile, follow the same turns, and omit the
// leader's final westward step to remain one tile behind.
step MOVE_WALK_EAST, 1
step MOVE_WALK_SOUTH, 2
step MOVE_WALK_WEST, 4
step MOVE_WALK_SOUTH, 4
step MOVE_WALK_WEST, 2
step_end
.close

// Mr. Pokémon: replace the obsolete Mystery Egg handoff with Shiny Bait,
// then continue through Oak's untouched Pokédex sequence.
.open "build/a012/2_229", 0
.if readu16("build/a012/2_229", 0x8B) == SCRIPT_OPCODE_SETVAR
    .if readu16("build/a012/2_229", 0x8D) != VAR_SPECIAL_x8004 || readu16("build/a012/2_229", 0x8F) != ITEM_MYSTERY_EGG
        .error "Revised opening found unexpected Mystery Egg parameters"
    .endif
.elseif readu16("build/a012/2_229", 0x8B) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_229", 0x8D) != (mr_pokemon_gives_shiny_bait - 0x91)
        .error "Revised opening found an unexpected existing Mr. Pokemon branch"
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
goto mr_pokemon_gives_shiny_bait

// The assistant now gives the Fishing Rod at Elm's lab. Restore Oak's original
// close-and-depart sequence so this patch is also correct after rebuild_scripts
// has read a previously patched member.
.org 0x34F
closemsg
apply_movement MR_POKEMON_OAK_OBJECT_ID, 0x450

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
mr_pokemon_gives_shiny_bait:
goto_if_no_item_space ITEM_SHINY_BAIT, 1, mr_pokemon_shiny_bait_bag_full
setvar VAR_SPECIAL_x8004, ITEM_SHINY_BAIT
setvar VAR_SPECIAL_x8005, 1
callstd std_obtain_item_verbose
npc_msg 3
npc_msg 4
wait_button
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

mr_pokemon_shiny_bait_bag_full:
callstd std_bag_is_full
closemsg
releaseall
end
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

// Every story object representing the counterpart's companion uses static
// sprite tag 1032. Its graphics now resolve to Slakoth, so keep the four
// surviving companion cries consistent with that shared visual mapping.
.open "build/a012/2_093", 0
.if readu16("build/a012/2_093", 0x64) != SCRIPT_OPCODE_PLAY_CRY
    .error "Slakoth companion found an unexpected D37 cry command"
.endif
.if readu16("build/a012/2_093", 0x66) != SPECIES_MARILL && readu16("build/a012/2_093", 0x66) != SPECIES_SLAKOTH
    .error "Slakoth companion found an unexpected D37 cry species"
.endif
.org 0x66
.halfword SPECIES_SLAKOTH
.close

.open "build/a012/2_225", 0
.if readu16("build/a012/2_225", 0x1B6) != SCRIPT_OPCODE_PLAY_CRY
    .error "Slakoth companion found an unexpected Route 29 cry command"
.endif
.if readu16("build/a012/2_225", 0x1B8) != SPECIES_MARILL && readu16("build/a012/2_225", 0x1B8) != SPECIES_SLAKOTH
    .error "Slakoth companion found an unexpected Route 29 cry species"
.endif
.org 0x1B8
.halfword SPECIES_SLAKOTH
.close

.open "build/a012/2_842", 0
.if readu16("build/a012/2_842", 0x16C4) != SCRIPT_OPCODE_PLAY_CRY
    .error "Slakoth companion found an unexpected New Bark cry command"
.endif
.if readu16("build/a012/2_842", 0x16C6) != SPECIES_MARILL && readu16("build/a012/2_842", 0x16C6) != SPECIES_SLAKOTH
    .error "Slakoth companion found an unexpected New Bark cry species"
.endif
.org 0x16C6
.halfword SPECIES_SLAKOTH
.close

.open "build/a012/2_849", 0
.if readu16("build/a012/2_849", 0x1AC) != SCRIPT_OPCODE_PLAY_CRY
    .error "Slakoth companion found an unexpected Elm lab cry command"
.endif
.if readu16("build/a012/2_849", 0x1AE) != SPECIES_MARILL && readu16("build/a012/2_849", 0x1AE) != SPECIES_SLAKOTH
    .error "Slakoth companion found an unexpected Elm lab cry species"
.endif
.org 0x1AE
.halfword SPECIES_SLAKOTH
.close

// Route 31 gatehouse: make the counterpart battle mandatory before awarding
// the Vs. Recorder. Player gender 0 uses Lyra; gender 1 uses Ethan, matching
// the existing GenderMsgBox order in this scene.
.open "build/a012/2_231", 0
.if readu16("build/a012/2_231", 0x13D) == SCRIPT_OPCODE_GENDER_MSGBOX
    .if readu8("build/a012/2_231", 0x13F) != 2 || readu8("build/a012/2_231", 0x140) != 3
        .error "Gatehouse battle found unexpected counterpart messages"
    .endif
.elseif readu16("build/a012/2_231", 0x13D) == SCRIPT_OPCODE_GOTO
    .if readu32("build/a012/2_231", 0x13F) != (gatehouse_friend_battle - 0x143)
        .error "Gatehouse battle found an unexpected existing branch"
    .endif
.else
    .error "Gatehouse battle found an unexpected Vs. Recorder scene"
.endif
.org 0x13D
goto gatehouse_friend_battle

.org 0x4A0
gatehouse_friend_battle:
gender_msgbox 2, 3
closemsg
get_player_gender VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_eq gatehouse_battle_lyra
trainer_battle TRAINER_ETHAN_GATEHOUSE, 0, 0, 0
goto gatehouse_battle_finished
gatehouse_battle_lyra:
trainer_battle TRAINER_LYRA_GATEHOUSE, 0, 0, 0
gatehouse_battle_finished:
check_battle_won VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_ne gatehouse_battle_won
// This is a mandatory battle. A loss follows normal blackout handling and
// leaves the scene variable unchanged, so the player must return and win.
white_out
releaseall
end
gatehouse_battle_won:
GiveItemNoCheck ITEM_VS_RECORDER, 1
// This one-off story battle does not run the shared optional-trainer reward
// script, so award its configured Oran Berry directly on the victory path.
GiveItemNoCheck ITEM_ORAN_BERRY, 1
gender_msgbox 4, 5
// Resume at the original CloseMsg; the gift and message bytes displaced by
// the six-byte branch above have both been replayed here.
goto 0x155
.close
