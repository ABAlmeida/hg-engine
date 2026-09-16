.nds

.include "armips/include/scriptmacros.s"

// Route 42 gatehouse script header (member 498). Run script entry 2 whenever
// the map loads so the interception tile is entirely inactive after unlock.
.if filesize("build/a012/2_498") != 4
    .error "Unexpected Route 42 gatehouse script-header size"
.endif
.if readu32("build/a012/2_498", 0) != 0
    .error "Unexpected Route 42 gatehouse script header"
.endif

.create "build/a012/2_498", 0
map_script 2, 3
.byte 0
.align 4
.close
