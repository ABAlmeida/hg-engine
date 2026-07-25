.text
.align 2
.thumb

.global set_starter_hidden_ability_hook
set_starter_hidden_ability_hook:
mov r5, r2
add r4, #8
mul r5, r1
add r1, r4, r5
push {r0-r7}
bl set_starter_hidden_ability
pop {r0-r7}
ldr r2, =0x02096160 | 1
bx r2

.pool


.global ImplementLevelCap_hook
ImplementLevelCap_hook:
ldr r3, =0x206E540 | 1
bl bx_r3
// r0 is level
bl IsLevelAtLevelCap
cmp r0, #1
beq return_to_2245B3E // above level cap
ldr r3, =0x02245A32 | 1
bx r3

return_to_2245B3E:
ldr r3, =0x2245B3E|1
// fall through

bx_r3:
bx r3

.pool


// r0 is level
.global CapRareCandies
CapRareCandies:
bl IsLevelAtLevelCap
cmp r0, #0
bne return_to_208FEFA
mov r0, r4
ldr r3, =0x0201AB0C | 1 // free memory
bl bx_r3
ldr r3, =0x0208FEF4 | 1
bx r3

return_to_208FEFA:
ldr r3, =0x0208FEFA | 1
bx r3


// still needed because function itself is not 0x1C bytes long
.global Bag_HasSpaceForItem_hook
Bag_HasSpaceForItem_hook:
ldr r5, =Bag_HasSpaceForItem_return_address
mov r6, lr
str r6, [r5]
pop {r5-r6}
bl Bag_HasSpaceForItem
ldr r1, =Bag_HasSpaceForItem_return_address
ldr r1, [r1]
mov pc, r1

.pool

Bag_HasSpaceForItem_return_address:
.word 0


// still needed because function itself is not 0x1C bytes long
.global Bag_HasItem_hook
Bag_HasItem_hook:
ldr r5, =Bag_HasItem_return_address
mov r6, lr
str r6, [r5]
pop {r5-r6}
bl Bag_HasItem
ldr r1, =Bag_HasItem_return_address
ldr r1, [r1]
mov pc, r1

.pool

Bag_HasItem_return_address:
.word 0


// still needed because function itself is not 0x1C bytes long
.global CanUseItemOnMonInParty_hook
CanUseItemOnMonInParty_hook:
ldr r5, =CanUseItemOnMonInParty_return_address
mov r6, lr
str r6, [r5]
pop {r5-r6}
bl CanUseItemOnMonInParty
ldr r1, =CanUseItemOnMonInParty_return_address
ldr r1, [r1]
mov pc, r1

.pool

CanUseItemOnMonInParty_return_address:
.word 0

.global PartyMenu_ItemUseFunc_LevelUpLearnMovesLoop_Case6_hook
PartyMenu_ItemUseFunc_LevelUpLearnMovesLoop_Case6_hook:
mov r0, r4
bl PartyMenu_ItemUseFunc_LevelUpLearnMovesLoop_Case6
ldr r1, =0x02081ea6 | 1
bx r1

.global LevelToCap_UseItemOnMonInParty_hook
LevelToCap_UseItemOnMonInParty_hook:
bl LevelToCap_UseItemOnMonInParty
mov r0, r4
mov r1, #0xa1
ldr r3, =0x02081b00 | 1
bx r3

.global LevelToCap_ResumePartyMenu_hook
LevelToCap_ResumePartyMenu_hook:
push {r3-r5, lr}
mov r4, r0
bl LevelToCap_TryResumePartyMenu
cmp r0, #0
bge LevelToCap_ResumePartyMenu_return
ldr r0, =0x02081c51
mov r1, #0xc5
lsl r1, #4
add r1, #4
mov r2, #3
ldr r3, =0x02081eda | 1
bx r3

LevelToCap_ResumePartyMenu_return:
pop {r3-r5, pc}

.global LevelToCap_AfterEvolution_hook
LevelToCap_AfterEvolution_hook:
bl LevelToCap_IsActive
cmp r0, #0
beq LevelToCap_AfterEvolution_vanilla
mov r0, r4
mov r1, r5
bl LevelToCap_AfterEvolution
ldr r3, =0x0203db60 | 1
bx r3

LevelToCap_AfterEvolution_vanilla:
mov r0, r5
mov r1, #0xd6
lsl r1, #2
add r1, r4
ldr r3, =0x0203db34 | 1
bx r3

.global PartyMenu_ItemUseFunc_ReuseItem_hook
PartyMenu_ItemUseFunc_ReuseItem_hook:
mov r0, r4
bl PartyMenu_ItemUseFunc_ReuseItem
ldr r3, =0x02081398 | 1
bx r3

.global Bag_RenderMachineMoveSlot_hook
Bag_RenderMachineMoveSlot_hook:
ldr r3, [sp, #0x28]
sub sp, #8
str r3, [sp, #0]
mov r0, r5
mov r1, r6
mov r3, r4
bl  Bag_RenderMachineMoveSlot
ldr r3, [r4, #0]
cmp r0, #0
add sp, #8
ldr r2, =0x021FF662 | 1
bx  r2

.pool
