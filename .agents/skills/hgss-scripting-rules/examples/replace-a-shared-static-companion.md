# Replace a shared static companion Pokémon

## Goal

Change a recurring scripted companion's overworld species and keep its
scripted cries and source-controlled dialogue consistent without rewriting
each map event object.

## Target

- Graphics mapping: `src/field/overworld_table.c`, static sprite tag 1032
- Script archive/source: `armips/scr_seq/scr_seq_revised_opening.s`
- Script members with companion cries: 93, 225, 842, and 849

## Verified HGSS commands

- `PlayCry SPECIES, 0` is command 76. The parameter order is copied from the
  original companion uses in members 93, 225, 842, and 849: species first,
  then zero.
- The patch changes only the existing species halfword. It verifies both the
  command opcode and the old or already-patched species before writing.

## Verified identifiers

- Event banks 30, 35, 57, 94, 114, 115, 302, 335, and 341 use
  `SPRITE_FOLLOWER_MON_STATIC_MARILL`, numeric tag 1032, for the counterpart's
  recurring companion objects.
- `gOWTagToFileNum` maps tag 1032 to its actual Pokémon overworld graphics.
  Changing that single mapping to `SPECIES_SLAKOTH` updates every event object
  that shares the tag without editing opaque event members.
- `SPECIES_MARILL` and `SPECIES_SLAKOTH` come from the generated Armips
  species constants included by the script patch.
- Early source-controlled references are in message archives 379, 542, and
  545. Other NPC dialogue mentioning unrelated Marill must not be changed by
  a broad text replacement.

## Minimal pattern

```asm
.open "build/a012/SCRIPT_MEMBER", 0
.if readu16("build/a012/SCRIPT_MEMBER", COMMAND_OFFSET) != SCRIPT_OPCODE_PLAY_CRY
    .error "Unexpected companion cry command"
.endif
.if readu16("build/a012/SCRIPT_MEMBER", SPECIES_OFFSET) != SPECIES_OLD && readu16("build/a012/SCRIPT_MEMBER", SPECIES_OFFSET) != SPECIES_NEW
    .error "Unexpected companion cry species"
.endif
.org SPECIES_OFFSET
.halfword SPECIES_NEW
.close
```

## Control-flow checklist

- Do not change movement, object IDs, flags, script IDs, or event coordinates.
- Confirm the shared tag is not used by an unrelated static Pokémon before
  changing its graphics mapping.
- Guard every binary offset against the expected command and species.
- Audit companion-specific dialogue and cries separately; do not globally
  replace the species name in unrelated encounters or NPC text.

## Build and manual verification

When explicitly requested, build with `make quick-rom -j$(nproc)`. Inspect the
companion in New Bark, Route 29, the Route 31 gatehouse, and a later scripted
appearance. Confirm that it renders as Slakoth, plays Slakoth's cry, follows
the existing movements, and does not affect ordinary wild or trainer Marill.
