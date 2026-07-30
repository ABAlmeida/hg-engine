# Permanent Death and Wipe-Recovery Plan

## Intended behavior

After an eligible battle has copied its final party state back into the save:

- every non-Egg party Pokémon whose final HP is zero is permanently deleted;
- one or more removed Pokémon display
  `Your Pokémon that died have left the party.`;
- if no usable party Pokémon remains, the first non-Egg Pokémon in PC box and
  slot order is moved into the party;
- successful recovery additionally displays
  `{Pokémon} was moved from the PC and joined your party!`; and
- if no usable boxed Pokémon exists, the game displays the death notice,
  displays `You have no usable Pokémon remaining... Your journey is over.`,
  and returns to the title screen.

There is no separate retirement storage, Pokémon death flag, PC partition, or
special PC interface. All 30 expanded boxes remain ordinary usable storage.
Reloading the previous in-game save is explicitly allowed to undo deaths and
a wipe; the feature never forces an automatic save.

Final post-battle HP is authoritative. Direct damage, residual damage, recoil,
self-KO, and similar battle effects are treated identically. A Pokémon revived
before the battle finishes is retained. Eggs are never deleted and cannot
serve as wipe recovery.

HeartGold's overworld poison survival behavior currently stops at 1 HP, so it
does not trigger this policy. If field poison is later made lethal, its
completion path should call the same centralized operation.

## Implementation

### Central party mutation

`PermanentDeath_ProcessPartyAfterBattle` owns all deletion and recovery:

1. traverse the party backwards so compaction cannot skip adjacent entries;
2. delete every eligible zero-HP Pokémon;
3. record that a death notification is pending;
4. if the remaining party has no usable Pokémon, scan all PC boxes in
   deterministic box-major order;
5. copy the first non-Egg boxed Pokémon into a temporary party Pokémon;
6. add it to the party before deleting its boxed copy; and
7. retain its new party slot so the notification can format its nickname.

The add-before-delete ordering makes recovery transactional. Failure to add
leaves the boxed Pokémon untouched.

### Battle completion hook

The narrow hook at `0x02050724` replaces the original saved-party commit
wrapper without extending into the following encounter task. It:

1. preserves the original debug-battle exclusion;
2. calls the original save-copy operation exactly once;
3. skips temporary, link, facility, demonstration, and imported battle modes;
4. defers Bug-Catching Contest processing until its temporary party has been
   restored; and
5. runs the central mutation for ordinary persistent-party battles.

The Bug-Catching Contest command wrapper runs the same operation immediately
after `BugContest_Delete` restores the real party.

### Message timing

Battle tasks diverge after their shared commit point. For ordinary completion
paths, the project hooks the established `CallTask_FadeFromBlack` wrapper
rather than adding separate completion hooks to each encounter type.

When no notification is pending, the replacement starts and waits for the
same vanilla palette fade. When a notification is pending, a small wrapper:

1. waits until the restored overworld has faded in and is visible;
2. schedules an idle-dispatch task; and
3. returns to the original encounter task.

The dispatcher waits until the field TaskManager is empty before it starts
common field script 2074. This allows trainer and scripted encounter flows to
set their completion flags, show their normal post-battle dialogue, and release
the field before a new map-scene script begins. It also prevents the
notification from replacing a paused trainer script and accidentally leaving
that trainer undefeated.

The common script reads the pending-notification flag and recovered party slot
through the existing custom-script-command dispatcher, displays text from the
common message archive, and clears the pending state only after its dialogue
finishes. Recoverable wipes call the same shared body from the two vanilla
blackout recovery scripts immediately after their first fade-in. The
Bug-Catching Contest schedules the same idle dispatch after its temporary
party has been restored.

This keeps every notification on a visible overworld screen and avoids custom
window allocation, message formatting, and blackout rendering.

### No-reserve ending

Before a normal blackout can warp or restore the overworld, four narrow Thumb
call hooks check whether permanent-death processing found no usable party or
boxed Pokémon. A terminal wipe skips the death warp, replaces the vanilla
blackout text with `You have no usable Pokémon remaining... Your journey is
over.`, and waits for the normal blackout input and fade. It then skips
overworld restoration and the Pokémon Center/home script, clears the pending
state, and calls the established `OS_ResetSystem(0)` routine. The reset does
not depend on the current blackout task, its parent field script, or either
field application overlay completing a coordinated shutdown. Reset parameter
zero follows HGSS's normal intro/title startup path. It does not invoke credits
or save automatically.

Recoverable wipes retain the complete vanilla blackout path, followed by the
death and PC-recovery notifications in the restored field.

## Memory classification

- `permanent_death.c` and the custom-script-command cases consume the fixed
  injected code region.
- Two `u8` pending fields consume `.data`/`.bss` in that region, plus any
  linker-required alignment.
- Death and recovery text and the notification flow consume the common
  message/script NARCs, not injected code. The terminal blackout line consumes
  message archive 203.
- Notification windows use the established field-script message path.
- No save structure, save checksum layout, PC allocation, overlay heap, VRAM
  layout, or Pokémon structure is enlarged.

The implementation deliberately removes the previous storage UI, sandbox
copies, box reconciliation, navigation hooks, and 20/10 PC boundary. This is
both smaller and less fragile than preserving retired Pokémon in a custom UI.
The terminal path uses four call-site hooks in the central vanilla blackout
task; each retains the original behavior when the wipe is recoverable.

## Manual verification

Use a normal in-game save rather than a save state when changing ROM builds.
Verify:

1. one fainted Pokémon is deleted and produces the shared death notice;
2. multiple fainted Pokémon are all deleted and produce one shared notice;
3. surviving and revived Pokémon remain;
4. adjacent fainted party slots are not skipped;
5. a partial-party loss returns normally after the notice;
6. a wipe selects the first non-Egg boxed Pokémon in box/slot order;
7. the selected boxed copy is removed and its nickname appears in the second
   notice;
8. boxed Eggs are skipped;
9. a wipe with no usable reserve shows both ending notices and returns to the
   title;
10. ordinary wild, trainer, scripted, partner, double, and Bug-Catching
    Contest completion paths apply the policy;
11. excluded temporary/link/facility modes do not mutate the saved party;
12. all 30 PC boxes remain available through the ordinary PC interface; and
13. saving and loading after continued play retains the deletions and any
    reserve transfer.
