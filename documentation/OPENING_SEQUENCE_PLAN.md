# Revised Opening Sequence Plan

Last updated: 2026-08-01

## Status

Implemented in source. The main progression has passed an initial manual test;
the dialogue, New Bark object cleanup, revised Silver path, Fishing Rod gift, and
registered Healing Kit cleanup still need focused manual verification.

## Decisions

- Use a new save for this opening. Existing saves that have passed any of the
  affected scenes do not need migration.
- Preserve the normal HGSS counterpart selection. Lyra demonstrates catching
  for a male player; Ethan does so for a female player. Revisit this only if
  the separate forced-protagonist plan is retained.
- The rival is always named `Silver`. Do not display the rival naming screen.
- Professor Elm continues to give the Healing Kit during the starter sequence.
- Elm's assistant gives 5 Potions, 20 standard Poké Balls, and 20 Poké Bait in
  one conversation before the player leaves the lab.
- Move Silver's first battle to immediately after the player leaves Elm's lab.
- Preserve Mum's automatic first-downstairs cutscene. Extend that same
  cutscene so Mum gives the normal opening menu features, the Pokégear, the
  Pokégear Map, and Running Shoes, and starts the existing savings
  conversation.
- The Cherrygrove guide tour does not run. Put the guide into his established
  post-tour state because the Map and Running Shoes are already unlocked.
- Mr. Pokémon gives the actual hatchable story Egg. It is initially Togepi;
  changing its species is deferred.
- Preserve the later post-Falkner Violet City assistant event, but replace its
  duplicate Egg with exactly one Shiny Bait. Rewrite both Elm's preceding
  phone call and the assistant's dialogue to describe the new reward.
- Preserve Professor Oak's Pokédex sequence at Mr. Pokémon's house.
- Professor Oak also gives the Fishing Rod before leaving Mr. Pokémon's
  house. Its internal item ID remains `ITEM_OLD_ROD` for compatibility.
- On leaving Mr. Pokémon's house, Elm calls to say that he has the data he
  needs and asks the player to hatch and care for the Egg. The player does not
  return to Elm.
- Keep the catching tutorial on the first outbound journey. Retain the
  counterpart and Marill's first overworld grass animation, replace its
  explanation with Poké Bait instructions, and skip the simulated capture
  battle.
- Do not give additional Poké Balls from the shortened tutorial.
- After Mr. Pokémon's sequence, the northern Route 30 path is immediately
  available and the player can continue toward Violet City.

## Intended progression

1. The player goes downstairs and Mum's normal automatic cutscene begins.
2. During that one cutscene, Mum completes the normal opening setup, unlocks
   the Pokégear Map and Running Shoes, and asks whether to save the player's
   money.
3. The player visits Elm and chooses a starter through the normal selection
   and nickname flow.
4. Elm gives the Healing Kit and registers his phone number.
5. The assistant gives 5 Potions, 20 Poké Balls, and 20 Poké Bait.
6. When the player leaves the lab, Silver approaches and starts Silver 1.
7. The first trip through Route 29 triggers the shortened counterpart and
   Marill tutorial. It explains Bait and Balls but does not start a tutorial
   battle.
8. Cherrygrove is already in its post-guide-tour state.
9. Mr. Pokémon gives the hatchable Egg. Oak gives the Pokédex and Fishing
   Rod.
10. Elm calls when the player leaves, asks them to hatch and care for the Egg,
    and releases them from the return objective.
11. Route 30's northern path is open and normal Violet City progression begins.
12. After Falkner, Elm directs the player to his assistant in Violet City's
    Poké Mart. The assistant gives one Shiny Bait instead of another Egg.

## Non-goals

- Do not implement the forced-female protagonist plan as part of this change.
- Do not replace the normal Lyra/Ethan counterpart selection.
- Do not choose a different story-Egg species yet.
- Do not implement the separate Laptop plan, even though Mum's revised
  conversation is its likely future gift point.
- Do not redesign Silver's first team, dialogue beyond what the new placement
  requires, or later rival encounters.
- Do not add a new save field solely for this sequence.
- Do not add C hooks for behavior that existing verified HGSS script commands
  and scene state can express safely.

## 1. Inventory the original state transitions

Before patching any script, inspect the target maps and export the complete
relevant script blocks and event data with DSPRE. The audit must cover:

- the player's house 1F and Mum's initial and later opening conversations;
- Elm's lab starter, Healing Kit, phone registration, assistant gift, police,
  and Mystery Egg return branches;
- New Bark Town's Silver objects and lab-exit triggers;
- Route 29's counterpart, Marill, catching-tutorial trigger, movements, and
  completion cleanup;
- Cherrygrove's guide, Running Shoes, Map, outdoor object, and indoor object;
- the original Silver 1 encounter and its cleanup near Cherrygrove;
- Mr. Pokémon's house, Mystery Egg, Oak, Pokédex, and Elm phone-call sequence;
- Route 30's battling-trainer blockage and the state that clears it; and
- the later Violet City assistant event, Elm's preceding phone call, and the
  legacy Egg flags and Violet scene values consumed by their downstream flow.

For every branch, record:

- script archive and entry;
- map header and event file;
- trigger coordinates and trigger type;
- NPC/object IDs and visibility flags;
- persistent flags and scene variables read or written;
- message bank and message IDs;
- standard scripts called;
- lock, movement, message, fade, battle, and termination lifecycle; and
- which later scripts consume each state value.

Do not infer an identifier from another map. Unknown archive, message, object,
flag, or variable IDs remain unresolved until verified in the target data.

Create or update focused how-to files under
`.agents/skills/hgss-scripting-rules/examples/` for the reusable patterns
established by the implementation. At minimum document fixed new-save names,
story-sequence redirection, shortening a tutorial before a task starts, and
moving a story Egg gift.

## 2. Add one guarded opening feature

Add one configuration switch for the revised opening and use it consistently
across script patches and the small new-save initialization described below.
When disabled, the original opening must remain intact, apart from independent
features such as the existing Healing Kit.

Keep one source-controlled patch owner per affected script member. Do not
apply two independent patches to the same offsets. In particular, coordinate
the revised Elm flow with
`armips/scr_seq/scr_seq_00843_elm_healing_kit.s`, which already extends script
member 843. Either extend that guarded patch cleanly or rename it into a
combined Elm-opening patch while retaining independent configuration guards.

Every fixed script offset must have:

- a descriptive named constant;
- an assertion for the expected original command and parameters;
- an idempotent assertion for an already-patched member where required;
- a named continuation point; and
- a comment explaining why that point is safe to leave and re-enter.

## 3. Consolidate Mum's opening conversation

Extend the existing automatic first-downstairs routine, script 000 in script
archive member 845. Do not remove its automatic trigger or modify player-house
header member 618. Preserve the routine's existing player and Mum movements,
music, dialogue, menu-feature grants, waits, message lifecycle, field lock,
and release.

After the existing menu-feature grants, continue within that same automatic
cutscene by reusing the exact commands and state writes from Mum's later
vanilla opening branches rather than reimplementing their effects.

The consolidated branch must:

1. preserve the normal initial menu-feature setup;
2. enable the Pokégear once through its established state;
3. register the Pokégear Map card rather than giving the legacy Town Map Bag
   item;
4. enable Running Shoes through the verified HGSS command;
5. run the existing Mum savings choice and preserve its yes/no behavior;
6. finish with `VAR_SCENE_PLAYERS_HOUSE_1F` set to 1, as in the original
   automatic routine, so New Bark's existing header can launch the normal
   initial Lyra/Ethan and Marill scene after the player leaves the house;
7. remain one-time when the player changes maps, saves, or reloads.

Do not advance the player-house scene directly to its terminal value during
this cutscene. Retire the later police/return-to-Mum sequence only after the
moved Silver 1 battle has completed, without suppressing the initial
counterpart scene.

Known repository evidence includes `FLAG_GOT_POKEGEAR`,
`FLAG_SYS_MOMS_SAVINGS`, `register_pokegear_card`, and
`give_running_shoes`. Their exact original parameter values and surrounding
control flow must still be copied from the verified HGSS scripts.

Do not include the planned Laptop gift in this implementation.

## 4. Preserve Elm's starter and Healing Kit flow

Retain the complete original starter selection, starter-dependent state,
nickname choice, and Elm dialogue. Keep the Healing Kit after the starter has
been received and before the player exits the lab.

Move or retain Elm's phone-number registration so it completes during this
same visit. Use the existing registration command and contact identifier from
the original Elm sequence. Do not invent a contact number or duplicate the
registration later.

Review the existing Healing Kit patch after the combined changes to ensure:

- every starter and nickname branch reaches the gift exactly once;
- the displaced Elm movement still occurs;
- message closure and movement waits remain balanced; and
- revisiting Elm cannot grant a second Healing Kit.

## 5. Give all starting supplies from the assistant

Extend the assistant's existing lab-exit conversation to award, in order:

1. 5 Potions;
2. 20 standard Poké Balls; and
3. 20 Poké Bait.

Use the established verbose item-obtained standard script for each stack so
the correct item name, quantity, fanfare, and pocket behavior are retained.
The opening starts with sufficient Bag capacity, so no new deferred-reward
save state is required.

Gate the complete assistant sequence with the existing starter/lab scene
progression. Advance its completion state only after all three grants, and
ensure the assistant's later dialogue cannot award them again.

Remove the normal later Poké Ball reward from the catching tutorial so the
agreed starting quantity is not silently increased.

## 6. Initialize the fixed rival name

Do not open the interactive `name_rival` screen. Initialize the existing
`SAVE_MISC_DATA.rivalName` field to `Silver` for new saves through the already
installed `Sav2_Misc_init_new_fields()` path.

This is preferred to a new script command because:

- the existing new-save hook already receives the correct save block;
- it writes the value once without a new persistent field or runtime hook;
- all later `buffer_rivals_name` calls continue to use the normal save value;
  and
- changing the fixed name later has one authoritative definition.

Before implementing the initializer, verify the game's character encoding
from the project charmap or a known-good encoded name. Do not assume ASCII or
UTF-16 values. Store the terminator and clear the unused portion of the
eight-code-unit field.

The fixed encoded name adds a very small constant to the injected code region.
Measure the resulting `save.o` change and linker headroom rather than
estimating it as free.

## 7. Move Silver 1 to New Bark Town

Reuse the existing New Bark rival object. Hide the original New Bark
counterpart and Marill objects before starting the replacement scene, matching
the cleanup performed by the displaced vanilla post-starter scene.

Keep Silver at his original window position `(682, 391)`. He runs south three
tiles in total, but detours west through x=681 to avoid the mailbox event at
`(682, 393)`, then runs east to face the player from `(683, 394)` and starts
the battle. Afterward he runs south two tiles and west seven tiles to the
verified Route 29 boundary at `(676, 396)`. This avoids both the lab footprint
and mailbox while keeping his starting position and departure direction visible.

Set the New Bark counterpart and Marill hide flags inside Elm's lab after the
assistant's successful supply gift, before the exterior map loads. The outdoor
scene repeats the cleanup defensively. Redirect New Bark's vanilla stage-1 map
setup to an empty routine, because that setup explicitly clears the same flags
and shows both objects on initial entry and on the post-battle field reload.

Trigger the sequence after the player leaves Elm's lab with a starter and
after the assistant's supplies have been awarded. Reuse the original Silver 1
logic for:

- selecting trainer 495, 496, or 497 from the player's starter;
- rival battle music and transition;
- battle outcome handling;
- post-battle dialogue and movement; and
- the persistent state consumed by later Silver encounters.

Winning continues to raise the cap to level 13 through the existing trainer-ID
table. Do not call level-cap logic directly from the field script.

All three Silver 1 trainer variants use one level-5 starter with 31 IVs in all
six stats and no held item. Their moves are fixed to non-STAB opening moves:
Chikorita has Tackle and Growl, Cyndaquil has Tackle, Leer, and Smokescreen,
and Totodile has Scratch and Leer. Explicit moves prevent Razor Leaf or Water
Gun from entering this first battle through the generated learnsets.
After the victory dialogue, award the player one Oran Berry and one IV Max
through consecutive uses of the standard verbose item-obtained flow. These
immediate script rewards are the temporary source of truth until the central
trainer reward system is implemented; migrate trainer IDs 495, 496, and 497 to
its external reward table and remove the scripted awards in the same change to
avoid duplicate rewards.

Launch Silver 1 without the trainer battle's `can lose` parameter. That
parameter restores and fades in the overworld after a defeat, which conflicts
with starting a normal blackout afterward. On a loss, call the standard
`white_out` command before releasing the field lock, matching the shared
trainer script so permanent-death wipe recovery or the no-reserve ending owns
the transition from the battle's black return screen.

After this sequence:

- `FLAG_MET_PASSERBY_BOY` and any verified companion scene state reflect a
  completed first meeting;
- the rival naming/police branch is already retired;
- the original Cherrygrove-area Silver 1 trigger cannot run; and
- later rival battles still use their existing Silver trainer IDs and the
  saved fixed name.

Permanent-death behavior remains active. Losing this first mandatory trainer
battle may end the run under the existing no-reserve wipe rule; this plan does
not add an opening exemption.

## 8. Keep a shortened Poké Bait tutorial

Make the existing Route 29 counterpart tutorial available on the first
outbound journey after Silver 1. Preserve normal counterpart selection:

- Lyra for a male player;
- Ethan for a female player; and
- the existing Marill object and movements for both paths.

Replace the catching explanation with dialogue that tells the player to:

1. stand on valid encounter terrain;
2. use Poké Bait to begin a wild encounter; and
3. throw a Poké Ball during the encounter.

Retain the first verified overworld movement in which the counterpart and
Marill move or jump into the grass. Branch around the `catching_tutorial`
command before opcode 251 starts `SetupAndStartTutorialBattle()`. Rejoin only
at a verified post-task continuation that does not expect tutorial-battle
state.

The shortened branch must still:

- wait for all retained movements;
- complete its messages cleanly;
- update the original one-time tutorial state;
- hide or reposition the counterpart and Marill correctly;
- release the field lock; and
- leave Route 29 traversal available.

Do not start a battle, allocate tutorial battle data, consume Bait or a Ball,
mark an encounter area consumed, update the Pokédex, or give more Poké Balls.

## 9. Put Cherrygrove into its post-tour state

Because Mum already grants the Map and Running Shoes, prevent the outdoor
guide tour from taking control of the player. Reproduce the verified
post-tour state rather than merely ending its trigger early.

Confirm that:

- the outdoor guide and its trigger cannot start the tour;
- the correct guide object is visible in his established later location;
- entering and leaving buildings cannot restore the outdoor event;
- the Map and Running Shoes remain enabled after save/reload; and
- any dialogue or item previously attached to the completed tour cannot be
  awarded twice.

## 10. Give the hatchable story Egg at Mr. Pokémon's house

Preserve Mr. Pokémon's introduction and Professor Oak's Pokédex sequence.
Replace the Mystery Egg key-item handoff with the established hatchable Egg
gift behavior.

Before choosing the Egg command, audit later scripts that read the saved
Togepi personality and gender:

- If retained later story checks require that identity, use the specialized
  Togepi Egg command after correcting its existing object-lifetime defect.
- If no retained consumer requires it, use the general verified Egg command
  with Togepi and the original story Egg's known-good parameters.

`ScrCmd_GiveTogepiEgg()` currently frees its temporary Pokémon and then reads
that freed object to save its personality and gender. If the specialized path
is retained, move the identity read/write before `sys_FreeMemoryEz()` and
review the generated object size. Do not use the current ordering.

The normal opening guarantees party space because the player has only their
starter. The implementation should nevertheless preserve the selected
command's established full-party behavior and must not claim the gift
succeeded if it did not.

Remove or retire the Mystery Egg key item so it cannot remain as an obsolete
quest item. Do not suppress the later Violet City assistant event: repurpose it
as the one-time Shiny Bait reward described below.

Changing Togepi to another species remains a later content change. Keep the
chosen species at one identifiable source location so that later work does not
require tracing several scripts or C literals.

## 11. Replace the panic call and retire the return journey

After the Egg and Pokédex sequences complete and the player leaves Mr.
Pokémon's house, use the established HGSS phone-call flow for a new Elm
message. Elm explains that he has the information he needs and asks the player
to hatch and care for the Egg.

Do not reuse an identifier or parameter until its original HGSS call sequence
has been verified. Preserve the call's field lock, music, message close,
phone-task completion, and release behavior.

At this point, reproduce the final persistent state of every skipped opening
sequence. The state audit must account for at least:

- Elm's panic call and return objective;
- the Mystery Egg delivery and Gym-journey authorization;
- the lab police scene;
- rival naming and Mum's post-rival branch;
- the original Silver 1 event;
- the moved and shortened catching tutorial;
- Cherrygrove's guide tour;
- Route 30's battling-trainer blockage; and
- the later Violet City assistant reward and its legacy Egg progression flags.

Set only values verified from the original scripts. Prefer the original final
scene values over a new parallel flag. Confirm all downstream consumers before
removing an intermediate state.

The Route 30 blockers must reach their normal post-opening state so the player
can continue north immediately. Do not merely disable collision or move an
object without advancing the story state that controls it on reload.

## 12. Repurpose the post-Falkner assistant gift

Preserve the established post-Falkner event that makes Elm's assistant appear
in Violet City's Poké Mart. The event must still run at its normal point after
Falkner, but its reward changes from the duplicate Togepi Egg to exactly one
`ITEM_SHINY_BAIT`.

Rewrite Elm's scripted post-Falkner phone message so he directs the player to
his assistant for a special reward rather than telling them to collect an Egg.
Rewrite the assistant's dialogue to say that Professor Elm sent the Shiny Bait
as a reward for looking after the Egg and to describe its intended use without
claiming that another Egg is being delivered. The final wording and message
IDs must be verified in the owning HGSS message banks during implementation;
do not invent a parallel C message.

Replace the assistant script's party-count check and `GiveTogepiEgg` operation
with the established HGSS Bag-space check and verbose item-award flow for one
Shiny Bait. Advance the event only after the item is successfully added. If
the Bag has no room, retain the assistant and the incomplete Violet scene so
the player can retry; do not award twice after a successful interaction.

Keep the original assistant appearance, movement, departure, visibility
cleanup, and downstream Violet and Elm scene transitions. Existing flags whose
names refer to receiving or collecting Elm's Egg may remain as legacy story
milestones where downstream scripts require them. Document that compatibility
meaning and set them at the verified points rather than adding a new save flag;
the Violet scene transition remains the authoritative one-time reward state.

## 13. Source-control dialogue and patch knowledge

Keep all new dialogue in the appropriate source-controlled message banks, not
as C strings. Preserve control codes and line endings used by the surrounding
bank. Validate message-bank compilation and review every modified message for
line length and speaker consistency.

Record a patch manifest containing each affected script member, message bank,
hook offset, original command assertion, appended routine, and continuation.
Generated DSPRE workspaces and installed script NARCs are not the source of
truth.

## Memory and performance

- Field scripts and dialogue consume script/message NARC and ROM space, not
  the fixed overlay-129 injected-code region. They use the normal temporary
  script/message heaps while their maps are active.
- The fixed `Silver` new-save initializer consumes a small amount of injected
  code and read-only data but no additional save space or runtime allocation.
- Reordering the existing Togepi helper's identity write should not require a
  new allocation or persistent field. Measure its object-size effect if used.
- Skipping the simulated tutorial battle removes that battle's temporary
  allocations and runtime cost from the opening.
- No graphics, palette, audio, or VRAM changes are planned.
- No new recurring overworld poll or per-frame hook is permitted.

After implementation, compare relevant object sizes, script-member sizes, and
remaining linker headroom. A ROM build alone does not prove that event state
or task ownership is correct.

## Fragility and maintainability requirements

- Prefer one coherent state-transition plan over independent flags added to
  make individual scenes disappear.
- Assert every fixed base-script command before replacing it.
- Keep each map's object IDs local to its patch and document their event-data
  provenance.
- Never jump into a continuation that expects a task, battle result, message,
  movement, or lock that the new branch skipped.
- Preserve one authoritative fixed rival name and one authoritative story-Egg
  species selection.
- Avoid adding save fields for one-time states already represented by vanilla
  scene variables or flags.
- Ensure `rebuild_scripts` remains idempotent and does not duplicate appended
  routines.
- Update this plan and the scripting examples if implementation evidence
  changes an assumed flow.

## Logical implementation steps

1. Export and document all affected original script blocks, event objects,
   triggers, messages, flags, variables, and downstream consumers.
2. Add the guarded revised-opening configuration and patch manifest.
3. Initialize the fixed rival name for new saves and verify its encoding.
4. Extend Mum's automatic downstairs cutscene with the Pokégear, Map, Running
   Shoes, and savings flow while preserving the initial New Bark counterpart
   scene.
5. Integrate Elm's phone registration with the existing starter and Healing
   Kit patch.
6. Add the assistant's three exact item stacks and remove the later tutorial
   Ball reward.
7. Move Silver 1 to New Bark and retire the original battle and naming flow.
8. Move and shorten the counterpart/Marill tutorial without starting opcode
   251's tutorial battle.
9. Advance Cherrygrove to its established post-tour state.
10. Move the hatchable Egg to Mr. Pokémon, resolve the Egg-helper lifetime
    issue, and retire the Mystery Egg key item.
11. Replace Elm's panic call and reproduce the verified final state of the
    skipped return journey.
12. Open Route 30 through its normal post-opening state.
13. Repurpose Elm's post-Falkner call and the Violet assistant gift as a
    one-time Shiny Bait reward with a Bag-full retry path.
14. Add the required scripting examples and update affected documentation.
15. Review all changes for script lifecycle, patch assertions, code size,
    readability, maintainability, and comment correctness.

Keep these steps separate enough to review, but do not produce intermediate
ROMs whose state model mixes incompatible halves of the old and new opening.

## Build and focused manual verification

After implementation, create the script-aware incremental ROM from MSYS2
UCRT64 with:

```sh
make quick-rom -j$(nproc)
```

The expected output is `test.nds`. Codex runs a Heartless Gold build only when
the user explicitly requests one. Heartless Gold tests and emulator use remain
prohibited by the project skill.

Use a new in-game save, not a save state, and manually verify:

1. Mum's complete automatic downstairs cutscene for both savings choices;
2. menu features, Pokégear, Map, and Running Shoes before visiting Elm;
3. no duplicate Mum conversation after map changes and save/reload;
4. the normal initial New Bark Lyra/Ethan and Marill scene after leaving home;
5. all three starters and both nickname choices;
6. exactly one Healing Kit and one Elm phone registration, plus Healing Kit
   registration and registered-button use;
7. exactly 5 Potions, 20 Poké Balls, and 20 Poké Bait after the assistant;
8. no duplicate gifts after revisiting the lab;
9. all three Silver 1 trainer variants using a level-5, perfect-IV starter with
   no held item, no STAB moves, and the Super Effective, Evaluate Attacks, and
   Expert Attacks AI modules, plus exactly one awarded Oran Berry, exactly one
   awarded IV Max, and the level-13 cap after a win;
10. the existing permanent-death outcome after losing Silver 1;
11. the saved rival name `Silver` in later dialogue after save/reload;
12. no police naming scene and no original Silver 1 encounter;
13. the correct Lyra/Ethan counterpart for each player gender;
14. the retained counterpart/Marill grass animation and new Bait explanation;
15. no tutorial battle, Ball gift, item consumption, Pokédex update, or area
    consumption from the shortened demonstration;
16. tutorial cleanup and one-time behavior after leaving and re-entering;
17. no Cherrygrove guide tour and the guide's correct post-tour placement;
18. Mr. Pokémon's hatchable Togepi Egg and Oak's Pokédex, phone, and Fishing
    Rod gifts, with no later duplicate Fishing Rod gift;
19. absence of an obsolete Mystery Egg key item;
20. the new Elm call completing without a lock, fade, or phone-task hang;
21. no return-to-Elm objective, police scene, or later Violet duplicate Egg;
22. immediate northern Route 30 access, including after save/reload;
23. normal Violet City, Falkner, and post-Falkner progression;
24. Elm's post-Falkner call referring to a special reward, not an Egg;
25. exactly one Shiny Bait from the Violet assistant, including a successful
    retry after a full-Bag refusal and no duplicate after save/reload;
26. the assistant's revised dialogue, departure, and normal downstream scene
    cleanup without granting a second Egg;
27. the Egg hatching and any retained Elm Egg dialogue; and
28. later Silver encounters displaying the fixed name and using their normal
    teams and progression.
