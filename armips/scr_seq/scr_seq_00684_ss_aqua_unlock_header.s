.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/vars.s"

// Olivine City script header (member 684). The player arrives from the Gym
// already standing on the exterior warp tile, so a coordinate event cannot
// fire until they leave and re-enter it. Add Elm's entry to the existing
// on-frame table instead. Transition entry 12 arms a map-local variable only
// after the Mineral Badge and while the ticket is unclaimed. Preserve both
// vanilla conditions ahead of the new entry.
.if filesize("build/a012/2_684") != 0x1C
    .error "Unexpected Olivine City script-header size"
.endif
.if readu32("build/a012/2_684", 0x01) != 13
    .error "Unexpected Olivine City transition entry"
.endif
.if readu32("build/a012/2_684", 0x06) != 1
    .error "Unexpected Olivine City on-frame table pointer"
.endif
.if readu16("build/a012/2_684", 0x0B) != VAR_UNK_4134
    .error "Unexpected Olivine City first on-frame condition"
.endif

.create "build/a012/2_684", 0
map_script 2, 13
map_script 1, olivine_frame_table - 0x0A
.byte 0

olivine_frame_table:
map_script_2 VAR_UNK_4134, 4, 3
map_script_2 VAR_UNK_40A5, 2, 4
map_script_2 VAR_TEMP_x400F, 1, 1
.halfword 0, 0, 0
.align 4
.close
