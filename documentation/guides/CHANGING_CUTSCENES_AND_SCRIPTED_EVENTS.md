# Changing Cutscenes and Scripted Events

This guide explains how HeartGold/SoulSilver field cutscenes are assembled,
how to find every resource involved in an event, and how to make a change
reproducible in Heartless Gold. The current Silver 1 scene in New Bark Town is
used as the main example.

Do not begin by inventing commands or guessing IDs. HGSS field scripting is a
version-specific bytecode interface. Every command, object, message, flag,
variable, trainer, map, and movement must be verified against an existing HGSS
use or the DSPRE HGSS command database.

## What a cutscene consists of

There is no single “cutscene file.” A scripted event can involve several map
resources:

1. **Map header** — selects the map's script, script-header, message, event,
   encounter, matrix, music, and other banks.
2. **Script header** — starts scripts on map transitions, map resume, or when a
   watched scene variable has a particular value.
3. **Script bank** — contains the commands, branches, calls, and movement lists.
4. **Message bank** — contains the dialogue referenced by that map's scripts.
5. **Event data** — defines NPCs, background events, coordinate triggers, and
   warps, including their coordinates and script IDs.
6. **Map geometry and collision** — determines whether a planned movement path
   is actually traversable.
7. **Persistent state** — flags and variables decide whether objects are shown
   and whether a one-time scene can run again.
8. **External content** — trainer teams, rewards, items, and other data may be
   stored outside the field script.

Changing only the visible dialogue or movement without tracing the other parts
can produce repeated scenes, NPC pop-in, invalid paths, post-battle hangs, or a
map that reloads into the wrong story state.

## Repository source of truth

Permanent changes must be represented by source-controlled files.

- `armips/scr_seq/*.s` contains reproducible patches and replacement scripts.
  These are Armips assembly sources, but most statements in these files encode
  HGSS field-script bytecode rather than ARM CPU instructions.
- `armips/include/scriptmacros.s` defines the exact byte layout of the HGSS
  script commands used by those patches.
- `armips/include/flags.s`, `armips/include/vars.s`, and the files under
  `asm/include/` provide verified named identifiers.
- `data/text/<bank>.txt` contains source-controlled message-bank text.
- `data/Trainers.c` contains trainer parties and trainer configuration. A field
  script chooses a trainer ID; it does not define that trainer's party.
- `.agents/skills/hgss-scripting-rules/examples/` records verified reusable
  scripting patterns.

The files under `.scratch/pokeheartgold-reference/` are useful read-only
reference material. They are not build inputs and must not become the only
place where a permanent change exists.

The generated files under `build/` are also not source. Armips patches them
during the build, after the pristine script archive has been extracted.

## Finding all resources for a map

Start with the map header. For New Bark Town, the reference map header records:

| Resource | New Bark Town value |
| --- | --- |
| Map | `MAP_NEW_BARK` / internal map name `T20` |
| Map ID | 60 |
| Script bank | member 842, `scr_seq_0842_T20` |
| Script-header bank | member 615, `scr_seq_0615_T20_hdr` |
| Message bank | member 542, `msg_0542_T20` |
| Event bank | member 57, `zone_event_057_T20` |

Useful reference files are:

```text
.scratch/pokeheartgold-reference/src/data/map_headers.h
.scratch/pokeheartgold-reference/files/fielddata/script/scr_seq/scr_seq_0842_T20.s
.scratch/pokeheartgold-reference/files/fielddata/script/scr_seq/scr_seq_0615_T20_hdr.s
.scratch/pokeheartgold-reference/files/fielddata/script/scr_seq/event_T20.h
.scratch/pokeheartgold-reference/files/fielddata/eventdata/zone_event/057_T20.json
```

For another map, find its map constant or internal map name in
`map_headers.h`, then record all four banks before editing anything.

## Understanding how the event starts

An event may be started by:

- talking to an NPC or background object;
- walking onto a coordinate trigger;
- entering or transitioning onto a map;
- resuming the map after a menu, battle, or other field task;
- an on-frame script-header entry watching a variable; or
- another script calling or branching to it.

New Bark's script header contains an on-frame table. When
`VAR_SCENE_NEW_BARK_TOWN_OW` equals `1`, it starts script entry 8. Heartless
Gold redirects that entry to `silver_one_after_lab` in
`armips/scr_seq/scr_seq_revised_opening.s`.

The same map has an on-resume script. Vanilla stage 1 shows the counterpart and
Marill again. Because a trainer battle unloads and restores field state, the
revised opening also redirects that setup entry. Without that second change,
the objects can reappear after Silver 1 even if the battle script hid them.

Before changing a trigger, answer all of these questions:

1. Which object, coordinate, transition, resume entry, or scene-variable entry
   calls the script?
2. What value makes it eligible to run?
3. Which command changes that value when the scene finishes?
4. What happens if a battle, menu, warp, or save/reload occurs during the flow?
5. Is another setup script showing or hiding the same objects?

## Inspecting objects and coordinates

Object IDs are local to a map. Never copy an object ID from a different map.
Inspect the target event data first.

Relevant New Bark objects include:

| Object | ID | Starting coordinate | Visibility flag |
| --- | ---: | --- | --- |
| Silver | 0 | `(682, 391)` | `FLAG_HIDE_NEW_BARK_RIVAL` |
| Marill | 3 | `(695, 406)` | `FLAG_HIDE_NEW_BARK_MARILL` |
| Counterpart | 4 | `(688, 392)` | `FLAG_HIDE_NEW_BARK_FRIEND` |

The event data also shows a mailbox/background event at `(682, 393)`, the
laboratory warp at `(684, 393)`, and the Route 29 boundary at x=`676`. This is
why Silver cannot simply run straight south from his window position.

Event data does not prove that every tile between two coordinates is walkable.
Use DSPRE's map and collision views to inspect the actual terrain. Write down
the route one tile at a time and account for:

- collision and elevation;
- warps and background events;
- the player and follower Pokémon;
- other NPCs that may be visible in the same story state;
- where the moving NPC must face before dialogue; and
- where an NPC can safely be hidden after departing.

If an NPC's starting position, sprite, trigger, or event flag must change, that
is an event-data change rather than merely a script change. The current
repository does not use the reference JSON as a build input, so establish a
source-controlled event-data workflow before relying on such an edit.

## Editing movement safely

The Silver movement lists currently live at these labels:

```asm
silver_approach:
step 17, 1
step 18, 1
step 17, 2
step 19, 2
step 35, 1
step_end

silver_depart:
step 17, 2
step 18, 7
step_end
```

These exact action values are already used by the manually checked scene. Do
not extrapolate other numeric movement actions from their values. Copy a named
movement from a known-good HGSS reference script or verify the required action
against HGSS movement definitions before introducing a different action.

`apply_movement object, list` starts a movement list. `wait_movement` waits for
the queued movement to finish. Multiple objects can be given movements before
one `wait_movement` when they are intended to move concurrently, as shown in
the vanilla New Bark scripts.

Every movement list must end with `step_end`. Omitting it allows the movement
interpreter to continue into unrelated bytes. Omitting `wait_movement` can let
dialogue, battle startup, object removal, or script termination occur while an
object is still moving.

Prefer changing the lengths or replacing existing steps when possible. Adding
steps increases the script-bank payload and may exceed the reserved region.

## Editing dialogue

Silver 1 uses message bank 542, stored at `data/text/542.txt`.

- Message 38 is the pre-battle line.
- Message 39 is the post-victory line.

Changing only the wording should normally require editing those existing lines,
not adding script commands or changing the message IDs. Message numbers are
zero-based: line 38 in the source is message ID 38.

HGSS text uses the project's character map. In particular, use the supported
curly apostrophe `’`; the ASCII apostrophe `'` is rejected by the validator.

The sequences `\n`, `\r`, and `\f` are message controls, not literal spacing.
Copy their use from a message with the desired, manually verified presentation.
Changing them can alter scrolling, paging, or input timing. Keep dialogue
within the DS text window and test every page in game.

Validate an edited bank without building the ROM:

```sh
. .venv/bin/activate
python3 tools/source/dumptools/validate_text_archive.py charmap.txt data/text/542.txt
```

Do not create a file containing only the changed message. A source-controlled
text bank replaces the complete bank, so every unchanged line must remain.

## Locks, messages, and task ownership

A typical automatic scene starts with `lockall` and ends with `releaseall`.
All terminal branches must balance that ownership unless a verified command is
deliberately taking over the field transition.

The current Silver 1 sequence uses verified commands in this order:

```asm
lockall
apply_movement SILVER_OBJECT_ID, silver_approach
wait_movement
callstd std_play_rival_intro_music
buffer_rivals_name 0
npc_msg 38
closemsg
```

Important rules:

- Do not display a message before its required string buffers are populated.
- Close a message before starting a battle or a movement that expects the
  message window to be gone.
- Use the wait paired with a started movement, fanfare, fade, or other
  asynchronous operation, copying a known-good HGSS example.
- Use `end` for a top-level script and the verified return command for a called
  subroutine. Do not substitute one for the other.
- Preserve unexplained setup commands such as the existing `scrcmd_609` unless
  their purpose and removal have been verified.

## Battles inside cutscenes

Silver 1 chooses one of three trainer IDs based on the player's starter:

| Player starter | Silver trainer ID |
| --- | ---: |
| Chikorita | 496 |
| Cyndaquil | 497 |
| Totodile | 495 |

The trainer parties themselves are defined in `data/Trainers.c`; see
`documentation/guides/CHANGING_TRAINER_POKEMON.md` before changing them.

The current battle call copies the ordinary single-trainer HGSS pattern:

```asm
trainer_battle TRAINER_ID, 0, 0, 0
check_battle_won VAR_SPECIAL_RESULT
compare VAR_SPECIAL_RESULT, 0
goto_if_ne silver_won
white_out
releaseall
end
```

The third battle parameter must remain zero for this scene. Setting the
`can lose` behavior caused the battle task to heal and restore the field before
the script attempted normal white-out handling, which previously produced a
soft lock.

Keep rewards exclusively on the victory branch. Silver 1 currently awards its
Oran Berry and IV Max there. A loss must not pass through dialogue, movement,
rewards, or completion-state changes intended for victory.

## Immediate state versus persistent state

`hide_person` changes the currently loaded object. The matching hide flag
controls whether that object returns when the map is loaded again. For a
one-time scene, both may be required:

```asm
hide_person SILVER_OBJECT_ID
setflag FLAG_HIDE_NEW_BARK_RIVAL
```

Silver 1 also sets `FLAG_MET_PASSERBY_BOY` and advances the relevant scene
variables. Those values are consumed by later story logic. Removing or moving
them without tracing every use can restore the original rival encounter,
repeat the new battle, show the wrong NPCs, or prevent later rival progression.

Before changing a flag or variable:

```sh
rg -n "FLAG_NAME|VAR_NAME" armips data documentation include src
```

Record every reader and writer. Temporary variables such as `VAR_TEMP_x4000`
are suitable for short-lived calculations; scene variables and flags are what
make progress survive map reloads and saving.

## Armips patch structure

The revised Silver scene redirects an existing script-table entry to code
placed later in member 842. Script-table pointers are relative, so the entry at
offset `0x20` stores:

```asm
.org 0x20
.word silver_one_after_lab - 0x24
```

The `0x24` value is the address immediately after the four-byte table entry.
Do not copy this arithmetic to a different entry without recalculating it.

Guard fixed offsets before writing them:

```asm
.if readu32("build/a012/2_842", 0x20) != ORIGINAL_POINTER && readu32("build/a012/2_842", 0x20) != (silver_one_after_lab - 0x24)
    .error "Unexpected New Bark script table entry"
.endif
```

The assertion accepts both the pristine and already-patched forms so deliberate
script rebuilds remain idempotent, while rejecting an unexpected archive.

The Silver replacement begins at `0x174C`, and `revised_new_bark_setup` is
fixed at `0x1840`. That makes the existing appended region tight. Adding
commands or movement steps without checking the assembled end can overlap the
setup routine. Dialogue-only edits in message bank 542 do not consume this
script space.

For a large rewrite, first design a verified relocation strategy and prove the
new region is unused. Do not guess that bytes which look empty are safe.

## Three safe levels of change

### 1. Text-only change

Use this when the event timing and choreography are already correct.

1. Edit the existing message in `data/text/<bank>.txt`.
2. Preserve every other message and its index.
3. Run the text validator.
4. Build and manually inspect every page.

This is the least fragile option and adds no script bytes.

### 2. Movement or command adjustment inside an existing replacement

Use this for small choreography changes such as Silver's approach or departure.

1. Confirm the object's starting coordinate in event data.
2. Plot the proposed route tile by tile in DSPRE.
3. Reuse verified movement actions.
4. Preserve `step_end`, `wait_movement`, locks, and all outcome branches.
5. Check the script's reserved byte range before increasing its size.
6. Test victory, loss, map reload, and save/reload behavior.

### 3. Trigger, object, or full-scene redesign

Use this only when changing when the event starts, which objects exist, or how
story progress is stored.

1. Trace the map header, script header, event data, script, and messages.
2. Identify every setup/on-resume routine touching the same objects.
3. Verify all flags and scene variables across the repository.
4. Establish source-controlled handling for every archive being changed.
5. Redirect the smallest existing entry rather than rewriting unrelated map
   scripts.
6. Add assertions for every fixed offset.
7. Document the verified pattern under
   `.agents/skills/hgss-scripting-rules/examples/`.
8. Test every trigger boundary and interruption path.

## Using DSPRE responsibly

DSPRE is useful for:

- finding the map and its event/script/message banks;
- viewing map geometry, collision, warps, NPCs, and coordinate triggers;
- inspecting existing HGSS commands and parameters; and
- prototyping a route or dialogue sequence.

Do not leave a permanent change only inside a DSPRE project or installed ROM.
Translate the result into the repository's source-controlled Armips, text,
data, or documented archive workflow.

`make rebuild_scripts` is only for deliberately importing edits from the
currently installed script archive. It is not required for ordinary changes
to `armips/scr_seq/*.s`, and it should not be used as a generic repair step.

## Review checklist

Before building, confirm:

- the correct map and all linked bank numbers were identified;
- every command was copied from a working HGSS use or verified against the
  DSPRE HGSS command database;
- every object ID came from the target map's event data;
- every message, trainer, item, flag, and variable ID was verified;
- movement paths avoid collision, warps, background events, and other actors;
- every movement has `step_end` and every required `wait_movement` remains;
- every lock has a release on all terminal paths;
- messages are buffered, closed, and paginated correctly;
- battle victory and loss take separate, complete paths;
- rewards cannot be granted after a loss or repeated after completion;
- immediate object visibility and persistent hide flags agree;
- map transition and on-resume scripts cannot restore removed objects;
- the completion state prevents the scene from repeating;
- appended bytecode cannot overlap another routine or exceed the member;
- fixed-offset patches have assertions; and
- a focused scripting example was added or updated when a script changed.

## Building and testing a cutscene change

From MSYS2 UCRT64, a normal source-controlled script/text change is included by:

```sh
make quick-rom -j$(nproc)
```

The output is `test.nds`. A successful build proves only that the archives were
assembled and packaged; it does not prove that the event's runtime control flow
is correct.

For Silver 1, manually verify at least:

1. the trigger runs once at the intended story state;
2. Silver is not briefly visible in an unintended position;
3. approach and departure paths avoid the mailbox, lab, player, and follower;
4. pre-battle dialogue and rival music occur in the intended order;
5. all three starter choices select the correct trainer;
6. victory shows the correct dialogue and awards each reward once;
7. loss follows permanent-death/white-out handling without hanging;
8. the counterpart and Marill do not return after the battle;
9. Silver remains hidden after leaving and after save/reload;
10. the original Silver 1 event cannot run later; and
11. later rival story events still recognize the completed first encounter.

Do not use a save state created during the old version of a cutscene to verify
new script startup or persistent state. Use an in-game save from before the
trigger, or a new save when earlier flags, variables, or object setup changed.
