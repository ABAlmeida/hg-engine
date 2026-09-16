.nds

.include "armips/include/vars.s"

// R42R0101 has no background events, one object, two warps, and no existing
// coordinate events. Append an eastbound interception tile inside the gate.
.if readu32("build/a032/zone_event_2_128", 0x00) != 0
    .error "Unexpected Route 42 gatehouse background-event count"
.endif
.if readu32("build/a032/zone_event_2_128", 0x04) != 1
    .error "Unexpected Route 42 gatehouse object-event count"
.endif
.if readu32("build/a032/zone_event_2_128", 0x28) != 2
    .error "Unexpected Route 42 gatehouse warp-event count"
.endif
.if readu32("build/a032/zone_event_2_128", 0x44) != 0
    .error "Unexpected Route 42 gatehouse coordinate-event count"
.endif

.open "build/a032/zone_event_2_128", 0
.org 0x44
.word 1
.org 0x48
// Script IDs in coordinate events are stored one-based, so 1 selects entry 0.
.halfword 1, 7, 7, 1, 1, 0, 0, VAR_TEMP_x400F
.close
