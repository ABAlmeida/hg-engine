# Reset after a terminal wipe

## Goal

Display the permanent-death journey-over message and reset the game without
depending on the active field script, task stack, or overlays to shut down.

## Target

- Script archive/source:
  `armips/scr_seq/scr_seq_00003_commonscript.s`
- Script label or entry: `_permanent_death_end_run`
- Trigger or caller: the shared permanent-death notification after
  `GetPermanentDeathShouldEnd` reports that no usable party or reserve remains

## Verified HGSS commands

- Command and parameters: `npc_msg 122`
  - Known-good repository use:
    `_permanent_death_end_run` in
    `armips/scr_seq/scr_seq_00003_commonscript.s`
  - Parameter order and meaning: message ID in common message archive 040
- Command and parameters: `GetPermanentDeathPending VAR_SPECIAL_x8000`
  - Known-good repository use: the shared permanent-death notification
  - Parameter order and meaning: destination script variable receiving zero
    when there is no pending notification and one when a notification is due
- Command and parameters: `wait_button`
  - Known-good repository use: immediately after permanent-death messages in
    the same shared notification
  - Parameter order and meaning: no parameters; waits for player confirmation
- Command and parameters: `closemsg`
  - Known-good repository use: immediately after each confirmed
    permanent-death message in the same shared notification
  - Parameter order and meaning: no parameters; closes the active message
- Command and parameters: `FinishPermanentDeathNotification 1`
  - Known-good repository definition:
    `armips/include/scriptmacros.s`, dispatched through
    `src/script_new_cmds.c`
  - Parameter order and meaning: the halfword argument is nonzero when the run
    must end
  - Implementation contract: clears transient notification state and calls
    `OS_ResetSystem(0)`

The reset behavior is verified from HGSS code rather than inferred from the
script-command name. `OS_ResetSystem(0)` is used from an active Frontier field
command in
`.scratch/pret-pokeheartgold/src/frontier/frontier_cmd_arcade.c`, and reset
parameter zero selects the normal intro/title startup path in
`.scratch/pret-pokeheartgold/src/main.c`.

## Verified identifiers

- Variables:
  - `VAR_SPECIAL_x8002` receives `GetPermanentDeathShouldEnd`.
- Flags: Not used.
- Messages:
  - Common message archive 040, message 121:
    `Your Pokémon that died have left the party.`
  - Common message archive 040, message 122:
    `You have no usable Pokémon remaining...\nYour journey is over.`
  - Blackout message archive 203, message 7 contains the equivalent
    terminal-wipe text for the vanilla blackout presentation.
- Trainer IDs: Not used.
- Map or event IDs: Not used.
- NPC or object IDs: Not used.
- Item or other content IDs: Not used.

## Minimal pattern

```asm
_permanent_death_end_run:
    npc_msg 122
    wait_button
    closemsg
    FinishPermanentDeathNotification 1
    releaseall
    end
```

The reset is expected not to return. `releaseall` and `end` remain defensive
script termination if that implementation contract changes later.

The shared notification checks whether any deaths are pending and displays
message 121 when the pending flag is nonzero. The message wording covers both
singular and plural removals, so no exact count is needed.

## Control-flow checklist

- The complete terminal message is rendered on one page, waits for
  confirmation, and closes before reset.
- No new script is started while another script owns the field TaskManager.
- The reset does not require the active script, child tasks, field overlay, or
  application overlay to finish first.
- The nonterminal branch passes zero and returns normally.
- The blackout path resets only after its replacement message closes and fades
  out; recoverable blackouts retain the vanilla warp and restoration flow.

## Build and manual verification

Build the source-controlled script, message, hook, and C changes with:

```sh
make quick-rom -j$(nproc)
```

Manually check trainer and wild terminal wipes, recoverable wipes with a boxed
Pokémon, and nonterminal post-battle death notifications. A successful build
does not prove that these runtime transitions behave correctly.
