# Replace a Contest with a permit session

## Goal

Reuse HGSS's Bug-Catching Contest field and candidate-comparison lifecycle
without preserving its weekday schedule, party reduction, judging, or prizes.

## Target

- Script archive/source: `armips/scr_seq/scr_seq_legendary_sanctuary.s`,
  patching extracted members 151, 242, 245, and script-header member 487.
- Zone-event archive/source: `armips/zone_event/legendary_sanctuary.s`,
  patching extracted members 99 and 101.
- Script entries: standard-script entries 0, 1, 4, 5, and 6; gatehouse
  init entries 12 and 0; gatehouse attendant entry 1 in both gates.
- Trigger or caller: Route 35/36 gate attendants, the active-session time and
  Ball exhaustion tasks, the interior retirement prompt, and the shared
  engine-owned Contest ending transition.

## Verified HGSS commands

All commands below follow working HGSS uses in pristine
`.scratch/pret-pokeheartgold/files/fielddata/script/scr_seq/scr_seq_0151.s`
and the Route 35/36 gate sources `scr_seq_0242_R35R0201.s` and
`scr_seq_0245_R36R0201.s`:

- `BugContestAction action, value`: byte action followed by a variable-resolved
  halfword; vanilla uses actions 0 and 1. The extension reserves action 2 as a
  read-only selected-pool eligibility query and returns its result through
  `VAR_SPECIAL_RESULT`.
- `ScriptOverlayCmd 1, state`: preserves the gate transition calls surrounding
  Contest lifecycle changes.
- `hasitem item, quantity, result` and `takeitem item, quantity, result`:
  preserve the verified parameter ordering used by repository item scripts.
- `yesno result`, `compare variable, value`, and `goto_if_* label`: the Yes/No
  result is zero for Yes and one for No, matching the existing Contest flow.
- `npc_msg id`, `wait_button`, and `closemsg`: all non-menu messages receive an
  explicit input wait before closure.
- `get_player_facing`, `apply_movement`, and `wait_movement`: the two gate
  approach wrappers ignore the outbound direction and move a refused player
  one tile away from the doorway. Their direction and movement constants are
  copied from the pristine gate scripts.
- `setflag`, `clearflag`, and `setvar`: use the same active-session, object, and
  gate variables as the original scripts.
- `fade_screen`, `wait_fade`, and `warp map, warp, x, z, direction`: preserve
  the verified warp contract. Script-owned endings clean up and warp directly
  to Route 35 gate map 102 at the verified attendant position. The shared
  engine Contest-ending transition is separately redirected to that gate, so
  National Park map 96 and standard-script entry 7 are never entered.
- `BugContestGetTimeLeft 0`: populates string variable 0 before either
  remaining-time retirement message. This is required even when the value is
  informational and follows the original menu and gate retirement scripts.
- `std_bug_contest_guard_start` is standard-script ID 10405 in HGSS
  `constants/std_script.h`. The standalone Armips environment does not import
  that symbolic header, so appended gate callbacks define a local named
  constant with value 10405 rather than referring to an undefined symbol.
- `ScrCmd_609` is deliberately not copied into Sanctuary ending entries. Its
  HGSS implementation checks the logical follower state and then assumes that
  partner object 0 exists. Full-party Sanctuary transitions do not preserve
  that map-object invariant, and the no-judging finish does not need the
  follower operation.
- command 21 (`RestartCurrentScript`): yields from the standard script back to
  its gate caller, as used by the pristine Contest standard script.

No command in the implementation was inferred from a different Pokémon
generation or an unverified DSPRE contract.

## Verified identifiers

- Variables/flags: `VAR_UNK_4118`, `VAR_UNK_40F7`,
  `FLAG_BUG_CONTEST_ACTIVE`, `FLAG_UNK_24E`, `FLAG_UNK_996`, attendant flags
  `FLAG_UNK_1C3`/`FLAG_UNK_1C4`, and contestant hide flags 627–636 come from
  pristine member 151 and the two gate scripts. `VAR_SANCTUARY_POOL_STAGE`
  aliases otherwise-unused `VAR_UNK_416E`; `FLAG_SANCTUARY_UNLOCKED` aliases
  otherwise-unused `FLAG_UNK_B4F` after repository-wide collision searches.
- Message IDs: standard-script member 151 imports text bank 246. The Sanctuary
  patch keeps IDs 78–87 as the ten original contestant names because the
  retained engine helper indexes those IDs directly; lifecycle text uses IDs
  0–26 from source-controlled `data/text/246.txt`.
- Map/event IDs: `MAP_D22R0102` is the Contest-variant National Park encounter
  map. Route 35 gate is map 102. Its always-enabled Sanctuary attendant is
  object 7 at (24, 3), and its north doorway is at (25, 2). A completed visit
  enters at the doorway and walks to (24, 4), directly in front of the
  attendant. Route 35 event member 99 adds a one-tile coordinate trigger at
  (25, 4); Route 36 event member 101 adds one at (2, 7). Both triggers use
  local attendant entry 1 and the verified temporary coordinate-event guard
  `VAR_TEMP_x400F == 0`.
- NPC/object IDs: the gate init entries manipulate only verified attendant
  flags. Contestant hide flags 627–636 are copied from member 151's original
  registration loop; no new object ID is invented.
- Item ID: `ITEM_SANCTUARY_PERMIT` is a new stackable Items-pocket ID appended
  after `ITEM_IV_MAX`. `ITEM_SANCTUARY_BALL` aliases the original Sport Ball ID
  so the battle ABI and supplied-Ball counter remain unchanged.
- Debug admission: `DEBUG_CHEATS` is exported from `include/debug.h` into the
  generated Armips configuration. At the start of the attendant's admission
  script, the debug-only branch sets `FLAG_SANCTUARY_UNLOCKED` when absent and
  uses the verified `hasitem`/`giveitem_no_check` pattern to award exactly one
  Permit only when the player currently has none. Release builds omit the
  entire branch.
- Hook address: the overlay-2 call to `BugContest_GetEncounterSlot` begins at
  `0x02247EE6`. The halfword at `0x02247EE4` is the preceding
  `movs r1, #4` argument setup and must not be registered as a `bl` hook.
- Trainers: Not used.

## Minimal pattern

```armips
admission:
    setvar VAR_UNK_4118, 0
    // An optional DEBUG_CHEATS block may set the unlock flag and award one
    // missing Permit here before the normal admission checks.
    goto_if_unset FLAG_SANCTUARY_UNLOCKED, locked
    BugContestAction 2, VAR_SANCTUARY_POOL_STAGE
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne refused
    hasitem ITEM_SANCTUARY_PERMIT, 1, VAR_SPECIAL_RESULT
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne refused
    // Perform storage and player-confirmation checks here.
    setflag FLAG_BUG_CONTEST_ACTIVE
    BugContestAction 0, VAR_SANCTUARY_POOL_STAGE
    compare VAR_SPECIAL_RESULT, 1
    goto_if_ne start_failed
    takeitem ITEM_SANCTUARY_PERMIT, 1, VAR_SPECIAL_RESULT
    setvar VAR_UNK_4118, 1
    restart_current_script

start_failed:
    clearflag FLAG_BUG_CONTEST_ACTIVE
    clearflag FLAG_UNK_24E
    // Restore the gate overlay/variables here, then refuse admission.
    goto refused

refused:
    // The unchanged gate caller treats 1 as permission to warp inside.
    setvar VAR_UNK_4118, 0
    restart_current_script

locked:
    // Display the locked message, leaving VAR_UNK_4118 at zero.
    restart_current_script
```

Before consuming a Permit, check the selected pool, the item, and the session
allocator's result. Remove the item only after those checks and the player's
Yes choice. If allocation fails, roll back the active flags and transition
state before refusing admission. An unlocked refusal must leave the gate result
at zero. Returning one from a refusal or inactive exit makes the unchanged gate
caller warp the player into the Sanctuary.

## Control-flow checklist

- Mark resident helpers called from a field extension with `LONG_CALL`; a
  normal far-call veneer can otherwise enter Thumb code in ARM state.

- Every branch restarts the gate caller or ends after an intentional warp.
- Inbound gate coordinate events call the same standard admission entry as a
  direct attendant conversation. A refusal or No choice walks the player one
  tile outward before releasing control, so the player cannot step from the
  trigger directly onto the park warp. Route 35 southbound and Route 36
  eastbound travel bypass admission, preserving ordinary exits.
- `lockall`/`releaseall` is balanced on time-up and completion paths.
- Non-menu messages wait for input before `closemsg`; Yes/No menus own their
  input themselves.
- Every fade is paired with `wait_fade`.
- One central teardown attempts party/PC storage, releases the candidate if
  neither accepts it, frees the session, clears all Contest lifecycle state,
  and always allows exit to continue.
- An inactive exit bypasses the formal retirement prompt and leaves the gate
  result at zero so the caller does not send the player back inside.
- Time-up, Ball-out, voluntary-retire, defeat, and battle-owned Ball exhaustion
  converge on central teardown. Script-owned paths warp directly after cleanup;
  engine-owned paths replace the original judging warp at its shared function
  entry. Script-owned paths run the arrival walk and thank-you dialogue after
  their warp. Engine-owned paths leave a one-time post-visit marker which the
  Route 35 gate's on-frame table consumes to run the same presentation.
- Sanctuary ending entries do not call `ScrCmd_609`; doing so can dereference
  a missing partner map object during a full-party Contest transition before
  the finish script reaches its dialogue or cleanup.
- The configurable duration is expressed in whole minutes. The Sanctuary
  initializes the original 20-minute elapsed counter with an offset, preserving
  the engine's field-only expiry check and remaining-time formatter. Time
  therefore cannot expire during a battle.
- Contestant object flags remain hidden; no daily completion flag is set.

## Build and manual verification

The Makefile assembles each `armips/scr_seq/*.s` patch independently before it
rebuilds the script archive. Do not include one of these standalone files from
`armips/global.s`: each file intentionally declares its own configuration and
constants, so assembling it in the global unit causes duplicate definitions.

Map trigger records belong to the separate zone-event archive. Assemble
`armips/zone_event/*.s` only after extracting `a/0/3/2`, rebuild that archive,
and install it back to `base/root/a/0/3/2`. Patching `build/a032` from a script
patch is insufficient: rebuilding `scr_seq.narc` only packs `build/a012`, so the
map-event edits would never reach the ROM.

Player-facing base-game references are changed with sparse records under
`data/text_patches/`. `tools/patch_text_messages.py` decodes only the named
message banks, verifies that each target still contains its expected original
phrase, replaces the selected record, and re-encodes the bank. This avoids
checking complete pristine dialogue banks into source control. Map/event and
script identifiers remain named for their original National Park/Contest data
so engine compatibility is not confused with player-facing terminology. NARC
member paths use three-digit indices (`7_062`, not `7_62`), while the JSON
keeps ordinary decimal bank keys for readability. The Windows `msgenc.exe`
text stream emits `CR-CR-LF` record separators because the encoder writes an
explicit CRLF through a translating text stream. Parse that exact separator
before ordinary CRLF/LF and never use Python `splitlines()`, which shifts the
message indices by treating both carriage returns as boundaries.

Executable hooks are installed by `scripts/make.py` before `armips/global.s`
runs. An Armips assertion against a hooked function's pristine prologue will
therefore inspect the already-patched instruction and fail even on the correct
base. Keep pristine hook-site verification in a pre-hook stage, or document the
source-derived address when no such stage exists.

Build with `make quick-rom -j$(nproc)` only when explicitly requested. A
successful build does not prove runtime script behavior. Manually verify:

- story-locked refusal;
- refusal without a Permit, and declining without entering;
- automatic interception from Route 35 and Route 36, including the outward
  one-tile turn-away movement and unobstructed travel out through both gates;
- content-not-ready and empty-stage refusal without Permit consumption;
- full-party preservation, final party/PC transfer, and release when both are
  full;
- Bait blocking in active and inactive visits;
- equal-weight selection and caught-species exclusions;
- time, Ball, and voluntary endings with no ranking, prizes, or old Contest
  NPCs;
- save/reload outside a session and ordinary battles elsewhere.
