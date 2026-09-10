.nds
.thumb

// Included by armips/global.s after build/armips_config.s.

.if IMPLEMENT_LEGENDARY_SANCTUARY

// scripts/make.py installs the Sanctuary_WarpToExit function-entry hook before
// this Armips unit runs, so base/arm9.bin no longer contains that site's
// pristine prologue here. The hook address is recorded and reviewed in hooks.

// BattleSetup_InitFromFieldSystem normally replaces the player's copied party
// with slot 0 whenever BATTLE_TYPE_BUG_CONTEST is set. The Sanctuary replaces
// the Contest completely, so take the existing full-party copy path instead.
.if readu16("base/arm9.bin", 0x51DEC) != 0xD00D && readu16("base/arm9.bin", 0x51DEC) != 0xE00D
    .error "Unexpected Bug Contest party-restriction branch"
.endif

.open "base/arm9.bin", 0x02000000

.org 0x02051DEC
.area 2
    b 0x02051E0A
.endarea

.close

.endif
