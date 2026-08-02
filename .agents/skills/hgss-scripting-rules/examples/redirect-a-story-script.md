# Redirect an HGSS story script safely

## Goal

Replace one story entry while preserving the map's existing trigger and event
data.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_revised_opening.s`
- Script entries: member 845 entry 0, member 842 entry 8, and member 858 entry 2
- Trigger or caller: their existing player-house, New Bark, and Violet scene
  values

## Verified HGSS commands and identifiers

The replacement routines copy command ordering from the corresponding vanilla
scripts in `.scratch/pret-pokeheartgold/files/fielddata/script/scr_seq/`.
Their table pointers, object IDs, scene variables, flags, and movement offsets
were verified against those scripts and their map event data before use. When
one replacement displaces another scene, preserve that scene's object cleanup:
the revised New Bark entry hides objects 4 and 3 and sets
`FLAG_HIDE_NEW_BARK_FRIEND` and `FLAG_HIDE_NEW_BARK_MARILL` before Silver is
shown.

For newly staged movement, verify every coordinate against the map and respect
the object's visible story position. Silver remains at the established window
position `(682, 391)`, runs south to `(682, 392)`, west to `(681, 392)`, south
to `(681, 394)`, then east to face the player from `(683, 394)`. This detour
passes west of z=393; a direct southward route crosses the mailbox event at
`(682, 393)`. His departure reaches the established New Bark west-exit boundary
at `(676, 396)` instead of disappearing southward.

Flags needed before a destination map appears must be set in the source-map
script before the warp. The revised assistant gift sets
`FLAG_HIDE_NEW_BARK_FRIEND` and `FLAG_HIDE_NEW_BARK_MARILL` while the player is
still inside Elm's lab. New Bark entry 9 must also be redirected under the
revised opening: its vanilla stage-1 setup explicitly clears those flags and
shows both objects, including when the field reloads after battle. Hiding them
only in the battle scene therefore causes both a visible pop-in and a post-
battle reappearance.

## Minimal pattern

```asm
.if readu32(file, TABLE_ENTRY) != ORIGINAL_POINTER
    .error "Unexpected script table entry"
.endif
.org TABLE_ENTRY
.word replacement - (TABLE_ENTRY + 4)
.org ORIGINAL_END
replacement:
// Verified commands copied from the original flow.
end
```

The stored pointer is relative to the byte immediately after its own table
entry. Accept an already-patched pointer as well when the patch must support
`make rebuild_scripts`.

## Control-flow and manual verification

Keep the original trigger and update its established terminal scene value.
Verify one-time behavior across map reload and save/load. Build with
`make quick-rom -j$(nproc)` when explicitly requested.
