set architecture armv5te
set endian little
set pagination off
set confirm off
set remotetimeout 10

set logging file .scratch/olivine-gdb.txt
set logging overwrite on
set logging redirect off
set logging enabled on

define hang-snapshot
    echo \n===== HANG SNAPSHOT =====\n
    info registers
    echo \n----- instructions around PC -----\n
    x/24i $pc-24
    echo \n----- stack memory -----\n
    x/32wx $sp
    echo \n----- backtrace -----\n
    bt
    echo ===== END SNAPSHOT =====\n
end
document hang-snapshot
Capture ARM9 registers, nearby instructions, stack memory, and a backtrace.
Run this after pressing Ctrl-C when the game is hung.
end

define data-abort-snapshot
    hang-snapshot
    set $fault_pc = $lr - 8
    echo \n----- data-abort fault site (LR_abt - 8) -----\n
    printf "fault_pc = 0x%08x\n", $fault_pc
    set arm force-mode thumb
    x/24i $fault_pc-24
    set arm force-mode auto
    info all-registers
end
document data-abort-snapshot
Capture a data abort, including Thumb disassembly at the faulting instruction.
Run this when breakpoint 3 at 0xffff0010 is hit.
end

define olivine-object-break
    break *0x0205ed90
end
document olivine-object-break
Add an optional breakpoint on HGSS map-object reconciliation.
This is intentionally not enabled by default because it is hit frequently.
end

target remote 127.0.0.1:3333

# Break if ARM9 enters an undefined-instruction, prefetch-abort, or data-abort
# exception vector. Map-object reconciliation is available separately through
# the olivine-object-break command because it is hit frequently during play.
break *0xffff0004
break *0xffff000c
break *0xffff0010

echo \nConnected. Exception breakpoints installed; continuing ARM9.\n
continue
