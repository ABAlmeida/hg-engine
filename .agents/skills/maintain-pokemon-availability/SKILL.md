---
name: maintain-pokemon-availability
description: Keep the generated Pokemon implementation, wild availability, and trainer-usage tracker synchronized. Use whenever Pokemon Heartless Gold species data, abilities, move implementation flags, learnsets, ordinary/Safari/Headbutt encounters, or trainer parties change.
---

# Maintain Pokémon Availability

Apply the repository's build and test restrictions. This is a source and
documentation workflow; it does not authorize a ROM build, game test, or
emulator launch.

## Required workflow

1. Make the requested source-data change. Never edit the generated species
   tables in `documentation/POKEMON_AVAILABILITY.md` by hand.
2. When a source-proven ability or species limitation is discovered, add its
   reason and evidence path to `data/pokemon_availability_overrides.json`.
   Do not classify an ability as unimplemented solely because a text search
   finds no dedicated handler; generic engine paths may implement it.
3. Regenerate the tracker from the repository root:

   ```sh
   python tools/generate_pokemon_availability.py
   ```

   This also regenerates the marked unusable-TM section in
   `documentation/TM_REWARD_AVAILABILITY.md`.

4. Confirm it is current:

   ```sh
   python tools/generate_pokemon_availability.py --check
   ```

5. Review the affected species rows. Verify status reasons, enabled wild
   locations, every trainer ID, summary counts, and alternate-form handling.
6. Run `git diff --check` and review the complete resulting diff while
   preserving unrelated working-tree changes.

## Classification rules

- Treat `FLAG_UNUSABLE_UNIMPLEMENTED` in `data/Moves.c` as authoritative for
  move implementation status. Categorize each affected learnset entry as
  level-up, TM, tutor, or Egg move.
- Only an unimplemented level-up move restricts the Pokémon's readiness.
  Unimplemented TM, tutor, and Egg moves are optional compatibility notes and
  must not lower the species status.
- Every TM item mapped to an unimplemented move must appear as `Unusable` in
  the generated section of `documentation/TM_REWARD_AVAILABILITY.md`; never
  offer it as a reward candidate.
- Treat an ability limitation as authoritative only when the repository says
  it is unimplemented or the limitation has been demonstrated and recorded in
  the override file with evidence.
- Keep source limitations distinct from runtime verification. A `Ready` row
  means no known blocker was found; it does not prove every mechanic works.
- Keep National Pokédex numbers separate from engine species IDs. HGSS engine
  IDs 494-543 are Egg/Bad Egg and numbered placeholders, not Generation V
  Pokémon; exclude them and derive later Dex numbers from the ordered real
  species constants.
- A zero encounter-method rate is not catchable content even when stale slots
  remain populated. Tables whose identifiers contain `UNUSED` are excluded.
- Keep globally unobtainable encounter methods in the override file's
  `disabledEncounterMethods` list so populated but unreachable Good/Super Rod
  tables are not reported as catchable content.
- Do not infer scripted gifts, trades, fossils, roamers, or one-off static
  encounters from arbitrary `SPECIES_*` references. Those paths require a
  separate verified source audit.
