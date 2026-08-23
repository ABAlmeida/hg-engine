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

Mum's combined opening in member 845 also preserves the original Pokégear
question contract. Message 10 contains the Yes/No control, but displaying it
does not read the selection: the verified vanilla flow hides the touchscreen
menu, calls `getmenuchoice VAR_SPECIAL_RESULT`, restores the touchscreen menu,
and selects message 11 for Yes or message 12 for No. Both paths converge on
message 13 and the original `wait_button_or_dpad`. The added Map and Running
Shoes messages also use explicit waits before the next gift or savings prompt;
a fanfare delay is not a substitute for player acknowledgement.

The same rule applies to text-controlled pauses in the retained Oak sequence
at Mr. Pokémon's house. Local message 12 in `data/text/377.txt` is followed
immediately by `GivePokedex` and its fanfare, so the message must end in the
verified HGSS `\r` acknowledgement control. Without it, the script advances as
soon as the printer finishes and the acquisition line can disappear without
player input. Oak's modified new-game messages 6, 34, and 35 in
`data/text/219.txt` already end in `\r` and therefore require acknowledgement.

Mr. Pokémon's revised gift follows the same boundary rule. Local message 2 is
immediately followed by `std_obtain_item_verbose`, and message 3 is immediately
followed by message 4. Messages 2 and 3 therefore end in `\r`; message 4 uses
the explicit `wait_button` already present in the redirected script before the
window closes and the healing transition begins.

Member 842 has very little appended space. After the rival-outro cleanup was
added, the Silver approach list fills `0x182C` through `0x1843`, and
`silver_depart` begins at `0x1844`. A former `.org 0x1840` setup stub
overwrote the departure's first movement action in the earlier layout even
though the source labels looked separate. The no-op setup now points to the
existing `end` terminating the Silver victory routine. Reusing a verified
terminator avoids both the overlapping write and an unnecessary extra command.

The first New Bark counterpart scene uses the same redirect-and-replay pattern
for a dialogue insertion. At member-842 offset `0xDCC`, Slakoth begins movement
`0xE60` immediately before the counterpart leads it away. The patch verifies
that command and object 3, replaces it with a branch, displays gendered local
messages 40/41, replays the displaced movement, and returns to the untouched
`wait_movement` at `0xDD4`. Returning to `0xDD4` is essential: returning to the
hook would repeat the dialogue, while returning after the wait would let the
departure start before Slakoth finishes moving.

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

After assembly, verify that each appended movement sequence still exists in
the output and that no later `.org` writes inside its byte range. Source order
alone does not prevent Armips from seeking backward and replacing prior data.

For a mid-script dialogue insertion, branch over the complete displaced
command, replay it in the appended continuation, and return to the original
command immediately following it. Assert the original opcode, object, and
relative target, plus the already-patched branch target for idempotence.

For embedded Yes/No text, copy the complete menu flow rather than only its
messages:

```asm
npc_msg QUESTION_MESSAGE
touchscreen_menu_hide
getmenuchoice VAR_SPECIAL_RESULT
touchscreen_menu_show
compare VAR_SPECIAL_RESULT, 0
goto_if_ne no_answer
npc_msg YES_MESSAGE
goto answer_done
no_answer:
npc_msg NO_MESSAGE
answer_done:
npc_msg FINAL_MESSAGE
wait_button_or_dpad
```

## Control-flow and manual verification

Keep the original trigger and update its established terminal scene value.
Verify one-time behavior across map reload and save/load. Build with
`make quick-rom -j$(nproc)` when explicitly requested.
