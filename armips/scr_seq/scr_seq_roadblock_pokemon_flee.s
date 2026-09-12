.nds

.include "armips/include/scriptmacros.s"
.include "armips/include/flags.s"
.include "build/generated/armips_species.s"

SCRIPT_OPCODE_SETFLAG equ 30
SCRIPT_OPCODE_WILD_BATTLE equ 589

ROUTE_36_SUDOWOODO_OBJECT_ID equ 4

ROUTE_11_SNORLAX_OBJECT_ID equ 4
ROUTE_11_SNORLAX_CHILD_1_OBJECT_ID equ 7
ROUTE_11_SNORLAX_CHILD_2_OBJECT_ID equ 8
ROUTE_11_SNORLAX_CHILD_3_OBJECT_ID equ 9

ROUTE_12_SNORLAX_OBJECT_ID equ 15
ROUTE_12_SNORLAX_CHILD_1_OBJECT_ID equ 16
ROUTE_12_SNORLAX_CHILD_2_OBJECT_ID equ 17
ROUTE_12_SNORLAX_CHILD_3_OBJECT_ID equ 18

.macro assert_roadblock_hook,file,hook,hide_flag,species
    .if readu16(file, hook) != SCRIPT_OPCODE_SETFLAG
        .error "Unexpected roadblock encounter hook opcode"
    .endif
    .if readu16(file, hook + 2) == FLAG_ENGAGING_STATIC_POKEMON
        .if readu16(file, hook + 4) != SCRIPT_OPCODE_WILD_BATTLE || readu16(file, hook + 6) != species
            .error "Unexpected roadblock wild battle"
        .endif
    .elseif readu16(file, hook + 2) != hide_flag
        .error "Unexpected roadblock encounter hook flag"
    .endif
.endmacro

// Route 36: both branches shake Sudowoodo after the SquirtBottle message.
// Replace the battle transition with a permanent disappearance, then either
// resume the original flower-girl reward scene or end the postgame retry path.
assert_roadblock_hook "build/a012/2_243", 0x145, FLAG_HIDE_ROUTE_36_SUDOWOODO, SPECIES_SUDOWOODO
assert_roadblock_hook "build/a012/2_243", 0x2F7, FLAG_HIDE_ROUTE_36_SUDOWOODO, SPECIES_SUDOWOODO

.open "build/a012/2_243", 0

.org 0x145
    setflag FLAG_HIDE_ROUTE_36_SUDOWOODO
    hide_person ROUTE_36_SUDOWOODO_OBJECT_ID
    // This flag prevents the Hall of Fame reset from respawning Sudowoodo.
    setflag FLAG_CAUGHT_SUDOWOODO
    goto 0x183

.org 0x2F7
    setflag FLAG_HIDE_ROUTE_36_SUDOWOODO
    hide_person ROUTE_36_SUDOWOODO_OBJECT_ID
    setflag FLAG_CAUGHT_SUDOWOODO
    releaseall
    end

.close

// Snorlax is represented on both Route 11 and Route 12. Hide the active map's
// Snorlax and crowd immediately, and set both persistent visibility flags so
// approaching the roadblock from the other map cannot start the old battle.
assert_roadblock_hook "build/a012/2_197", 0x77, FLAG_HIDE_ROUTE_11_SNORLAX, SPECIES_SNORLAX

.open "build/a012/2_197", 0

.org 0x77
    setflag FLAG_HIDE_ROUTE_11_SNORLAX
    setflag FLAG_HIDE_ROUTE_12_SNORLAX
    hide_person ROUTE_11_SNORLAX_OBJECT_ID
    hide_person ROUTE_11_SNORLAX_CHILD_1_OBJECT_ID
    hide_person ROUTE_11_SNORLAX_CHILD_2_OBJECT_ID
    hide_person ROUTE_11_SNORLAX_CHILD_3_OBJECT_ID
    // The caught flag suppresses the Hall of Fame respawn; it does not update
    // the Pokédex or add Snorlax to the party.
    setflag FLAG_CAUGHT_SNORLAX
    setflag FLAG_UNK_998
    setflag FLAG_SNORLAX_MEET
    releaseall
    end

.close

assert_roadblock_hook "build/a012/2_199", 0x275, FLAG_HIDE_ROUTE_12_SNORLAX, SPECIES_SNORLAX

.open "build/a012/2_199", 0

.org 0x275
    setflag FLAG_HIDE_ROUTE_11_SNORLAX
    setflag FLAG_HIDE_ROUTE_12_SNORLAX
    hide_person ROUTE_12_SNORLAX_OBJECT_ID
    hide_person ROUTE_12_SNORLAX_CHILD_1_OBJECT_ID
    hide_person ROUTE_12_SNORLAX_CHILD_2_OBJECT_ID
    hide_person ROUTE_12_SNORLAX_CHILD_3_OBJECT_ID
    setflag FLAG_CAUGHT_SNORLAX
    setflag FLAG_SNORLAX_MEET
    releaseall
    end

.close
