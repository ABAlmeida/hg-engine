# Legendary Pokémon Sanctuary

Last updated: 2026-09-10

## Goal

Replace the Bug-Catching Contest with a reusable Legendary Pokémon Sanctuary
session. After the story unlock, a consumable Sanctuary Permit admits the
player for one formal visit; declining or lacking a Permit leaves the player
at the gate. During a session the player may catch several eligible Legendary
Pokémon and retain exactly one.

## Decided rules

- The old Contest Pass is not required and is not repurposed.
- A stackable Sanctuary Permit starts one five-minute session. Duration,
  Ball count, catch-rate multiplier, and active walking rate are configurable.
  The duration remains minute-based and expires only after control returns to
  the field, never in the middle of a battle.
- The player enters with their complete real party. No party is copied,
  reduced, backed up, or restored by the Sanctuary.
- Ninety-nine Sanctuary Balls are supplied per visit. Their
  count and 1200% catch-rate multiplier remain content constants.
- The map's ordinary encounter table is empty. The active-session selector is
  the only source of encounters there.
- Poké Bait and Shiny Bait are blocked throughout the Sanctuary map, whether a
  session is active or not.
- All eligible species in the current stage pool have equal probability.
  Duplicate rows are ignored after the first so content mistakes cannot weight
  one species more heavily.
- Species already registered as caught are excluded.
- Intermediate candidates are held in the original Contest candidate slot,
  outside the party. Captures use the normal immediate Pokédex registration,
  so a caught species is subsequently excluded by the existing caught-species
  check even if the player later replaces that candidate.
- Only the final retained candidate enters the party when space exists and
  otherwise goes to the first free PC slot. If both are full at teardown, the
  candidate is released and the player still exits normally.
- The old `bugContestPokemonKept` save byte remains in place for save-layout
  compatibility but is legacy state and is not read or written by Sanctuary
  sessions.
- There is no judging, ranking, opponent roster, score, participation prize,
  weekday restriction, or daily entry restriction. Every formal ending returns
  the player through the Route 35 doorway, walks them to the counter, and runs
  the attendant's closing message automatically.
- Walking toward either gate's Sanctuary doorway automatically starts the
  existing attendant admission flow. A refusal or No choice turns the player
  one tile away; outbound travel through either gate is never Permit-gated.
- A species that is knocked out may reappear in the same session for now.
  Session-only KO exclusion is a possible later enhancement.

## Content controls

The implementation exposes four replacement pools selected by
`VAR_SANCTUARY_POOL_STAGE`. Later stages replace earlier ones; they do not
accumulate. Each entry contains a species and minimum/maximum level.

Stage 0 is enabled with twelve level-40 species: Articuno, Zapdos, Moltres,
Raikou, Entei, Suicune, Regirock, Regice, Registeel, Uxie, Mesprit, and Azelf.
Stages 1–3 remain empty replacement pools. `SANCTUARY_CONTENT_READY` is true,
so the attendant may consume a Permit when the selected stage has at least one
eligible species.

Zapdos is included deliberately even though its level-65 Magnetic Flux is
currently unimplemented. A level-40 Sanctuary Zapdos does not initially know
that move, and its limitation remains visible in the generated Pokémon
availability tracker.

Admission checks the currently selected stage at runtime. An empty stage, or a
stage whose species are all already caught, refuses entry without consuming a
Permit.

`FLAG_SANCTUARY_UNLOCKED` is the public story hook. No existing story event is
silently repurposed to set it; the eventual Sanctuary-introduction scene must
do so explicitly.

## Storage and failure behavior

Admission does not require advance storage space. At teardown, the retained
candidate is added to the party or the first free PC slot. If neither accepts
it, the candidate is released. Session teardown and exit always continue, so
storage pressure cannot preserve stale Contest state or strand the player.

## Implementation layers

- Field extension: session allocation, equal-weight pool selection, and final
  candidate transfer.
- In-place ARM9 patch: use the normal full-party copy path for Sanctuary
  battles instead of the Contest's one-Pokémon path.
- In-place patches: encounter-rate override, Contest call adapters, Ball
  multiplier, empty map table, and renamed Ball presentation.
- Central ending hook: the shared Contest battle-exit transition tears down the
  session and warps directly to the Route 35 gate instead of judging. The
  Route 35 on-frame script then walks the player in from the north doorway and
  runs the attendant's completion dialogue. Script-owned endings perform the
  same arrival sequence directly after their warp.
- Scripts/map events/text: Permit admission, story-lock response, automatic
  gate interception from dedicated Route 35/36 coordinate events, direct
  no-judging exits, completion dialogue, the content-not-ready guard, and
  sparse player-facing name replacements. The script and zone-event archives
  are rebuilt and installed independently.

## Required manual verification

### Issues addressed in source; build and manual verification pending

- Despite the intended full-party rule, a formal Sanctuary battle displayed
  only the player's lead Pokémon. The original cause was the explicit
  `BATTLE_TYPE_BUG_CONTEST` party-size branch; the Sanctuary now takes the
  existing full-party copy path instead.
- After successfully catching a Sanctuary Pokémon, the game hung on a black
  screen. The two optional custom layers around the original comparison UI
  (session-list recording and delayed Pokédex registration) have been removed,
  leaving the vanilla candidate handoff. Runtime verification is still needed.
- Refusing admission returned the gate's “enter” value, and inactive exits did
  the same, creating entry and re-entry loops. Refusal and inactive-exit paths
  now leave the gate result at zero.
- Timer, Ball, retirement, and defeat endings must all be verified to tear down
  the session and return directly to the Route 35 gate without loading the
  judging map or standard-script entry 7.
- The script-owned ending entries previously began with `ScrCmd_609`. That command
  assumes that an active follower has a corresponding partner map object, an
  invariant the full-party Sanctuary does not preserve during every Contest
  transition. The command was not needed by the no-judging finish path and has
  been removed from time-up, Ball-out, and retirement endings.
- Script-owned time-up, Ball exhaustion, and retirement paths use one shared
  cleanup block. Engine-owned battle endings use the original Contest's shared
  task transition site, redirected to the same teardown and a direct Route 35
  warp. Standard-script entry 7 and the judging map are not involved.
- Both retirement prompts call `BugContestGetTimeLeft 0` before displaying
  their remaining-time placeholder. Omitting that command left string variable
  0 uninitialised in the menu-launched path.
- The original Contest defeat task heals after requesting its transition.
  Central teardown processes permanent deaths before the redirected task
  returns, preserving the required ordering without a second defeat hook.

- Locked and content-not-ready admission do not consume a Permit.
- A session consumes exactly one Permit and supplies the configured Ball count.
- The player's 1–6 Pokémon party is unchanged on entry and exit.
- Inactive Sanctuary walking, Bait, and Shiny Bait cannot create encounters.
- Active encounters are equal-weight and use only the selected stage.
- Pokédex-caught species do not recur. Every successful capture is registered,
  including a candidate that is later replaced.
- The retained candidate reaches the party or PC when space exists; otherwise
  it is released without blocking teardown.
- Time expiry, Ball exhaustion, and voluntary exit skip judging and prizes,
  return to the Route 35 counter, and expose the closing dialogue there.
- Save/reload outside a session and ordinary wild/Contest-unrelated battles
  remain stable.
