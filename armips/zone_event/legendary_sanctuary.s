.nds

.include "build/armips_config.s"
.include "armips/include/config.s"
.include "armips/include/vars.s"

.if IMPLEMENT_LEGENDARY_SANCTUARY

// Add one-tile approach triggers in front of both park doorways. They reuse
// attendant entry 1, which the Sanctuary script patch redirects to its
// direction-aware admission wrapper. VAR_TEMP_x400F is the same temporary
// zero-valued coordinate-event gate used by pristine HGSS map events.
.if readu32("build/a032/zone_event_2_099", 0x194) != 1
    .error "Unexpected Route 35 park-gate coordinate-event count"
.endif
.open "build/a032/zone_event_2_099", 0
.org 0x194
.word 2
.org 0x1A8
.halfword 2, 25, 4, 1, 1, 0, 0, VAR_TEMP_x400F
.close

.if readu32("build/a032/zone_event_2_101", 0x64) != 0
    .error "Unexpected Route 36 park-gate coordinate-event count"
.endif
.open "build/a032/zone_event_2_101", 0
.org 0x64
.word 1
.org 0x68
.halfword 2, 2, 7, 1, 1, 0, 0, VAR_TEMP_x400F
.close

.endif
