.nds
.thumb

// Included by armips/global.s after build/armips_config.s.

.if SKIP_NEW_GAME_INFO_MENU

OAK_SPEECH_STATE_NO_INFO_NEEDED_FADE_IN equ 44

// At the end of Oak Speech state 0, US HeartGold normally advances to state
// 7 and opens the Control Info / Adventure Info / No Info Needed menu. State
// 44 is the existing No Info Needed fade-in path, so all later introduction,
// naming, save initialization, and field-entry states remain unchanged.
.if readu16("base/overlay/overlay_0053.bin", 0x17DC) != 0x2007 && readu16("base/overlay/overlay_0053.bin", 0x17DC) != 0x202C
    .error "New-game information-menu patch found an unexpected Oak Speech state transition"
.endif

.open "base/overlay/overlay_0053.bin", 0x021E5900

.org 0x021E70DC
.area 2
mov r0, #OAK_SPEECH_STATE_NO_INFO_NEEDED_FADE_IN
.endarea

.close

.endif
