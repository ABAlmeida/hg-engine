# TM Reward Availability

Last updated: 2026-08-22

This document tracks TM reward candidates and TM assignments that must not be
used because their moves are unimplemented. It is intended to support assigning
safe TMs as first-victory trainer rewards.

The machine assignments come from `src/item.c`. "Implemented" means the move is
not marked with `FLAG_UNUSABLE_UNIMPLEMENTED` in `data/Moves.c`; it does not
mean every interaction has been manually verified. Availability refers only to
the TM item. A move may still be available through level-up, breeding, a tutor,
or an HM.

## Status summary

| Status | Count |
|---|---:|
| Distinct implemented moves awaiting TM placement | 94 |
| Assigned as trainer rewards | 5 |
| Manually verified rewards | 0 |
| Unusable TM mappings | 12 |

## Updating this tracker

After changing move implementation flags or TM mappings, regenerate the
unusable-TM section and its summary count with:

```sh
python tools/generate_pokemon_availability.py
python tools/generate_pokemon_availability.py --check
```

When adding a TM to `data/trainer_rewards.csv`:

1. Confirm that the TM is in the candidate table and is not listed as
   `Unusable`.
2. Change **Availability** from `Unavailable` to `Available`.
3. Record the trainer name, numeric trainer ID, and location under **Reward source**.
4. Leave **Verification** as `Not verified` until the reward has been received
   and the TM has been used successfully in a current build.
5. After that manual check, change **Verification** to `Verified` and update the
   summary counts above.

Use one row per distinct move. Do not add another row merely because another TM
number teaches the same move.

## Candidate TM rewards

| TM item | Move | Engine status | Availability | Reward source | Verification |
|---|---|---|---|---|---|
| TM00 | Mega Punch | Implemented | Available | Sage Jin (ID 53), Sprout Tower, x1 | Not verified |
| TM095 | Leech Life | Implemented | Available | Youngster Albert (ID 49), Route 32, x1 | Not verified |
| TM096 | Eerie Impulse | Implemented | Unavailable | Unassigned | Not verified |
| TM097 | Fly | Implemented | Unavailable | Unassigned | Not verified |
| TM099 | Iron Head | Implemented | Unavailable | Unassigned | Not verified |
| TM100_SV | Dragon Dance | Implemented | Unavailable | Unassigned | Not verified |
| TM101 | Power Gem | Implemented | Unavailable | Unassigned | Not verified |
| TM102 | Gunk Shot | Implemented | Unavailable | Unassigned | Not verified |
| TM104 | Iron Defense | Implemented | Unavailable | Unassigned | Not verified |
| TM106 | Drill Run | Implemented | Unavailable | Unassigned | Not verified |
| TM108 | Crunch | Implemented | Unavailable | Unassigned | Not verified |
| TM109 | Trick | Implemented | Unavailable | Unassigned | Not verified |
| TM110 | Liquidation | Implemented | Unavailable | Unassigned | Not verified |
| TM112 | Aura Sphere | Implemented | Unavailable | Unassigned | Not verified |
| TM113 | Tailwind | Implemented | Unavailable | Unassigned | Not verified |
| TM117 | Hyper Voice | Implemented | Unavailable | Unassigned | Not verified |
| TM118 | Heat Wave | Implemented | Unavailable | Unassigned | Not verified |
| TM121 | Heavy Slam | Implemented | Unavailable | Unassigned | Not verified |
| TM122 | Encore | Implemented | Unavailable | Unassigned | Not verified |
| TM123 | Surf | Implemented | Unavailable | Unassigned | Not verified |
| TM124 | Ice Spinner | Implemented | Unavailable | Unassigned | Not verified |
| TM127 | Play Rough | Implemented | Unavailable | Unassigned | Not verified |
| TM128 | Amnesia | Implemented | Unavailable | Unassigned | Not verified |
| TM130 | Helping Hand | Implemented | Unavailable | Unassigned | Not verified |
| TM131 | Pollen Puff | Implemented | Unavailable | Unassigned | Not verified |
| TM132 | Baton Pass | Implemented | Unavailable | Unassigned | Not verified |
| TM133 | Earth Power | Implemented | Unavailable | Unassigned | Not verified |
| TM134 | Reversal | Implemented | Unavailable | Unassigned | Not verified |
| TM136 | Electric Terrain | Implemented | Unavailable | Unassigned | Not verified |
| TM137 | Grassy Terrain | Implemented | Unavailable | Unassigned | Not verified |
| TM138 | Psychic Terrain | Implemented | Unavailable | Unassigned | Not verified |
| TM139 | Misty Terrain | Implemented | Unavailable | Unassigned | Not verified |
| TM140 | Nasty Plot | Implemented | Unavailable | Unassigned | Not verified |
| TM142 | Hydro Pump | Implemented | Unavailable | Unassigned | Not verified |
| TM147 | Wild Charge | Implemented | Unavailable | Unassigned | Not verified |
| TM151 | Phantom Force | Implemented | Unavailable | Unassigned | Not verified |
| TM153 | Blast Burn | Implemented | Unavailable | Unassigned | Not verified |
| TM154 | Hydro Cannon | Implemented | Unavailable | Unassigned | Not verified |
| TM155 | Frenzy Plant | Implemented | Unavailable | Unassigned | Not verified |
| TM156 | Outrage | Implemented | Unavailable | Unassigned | Not verified |
| TM159 | Leaf Storm | Implemented | Unavailable | Unassigned | Not verified |
| TM160 | Hurricane | Implemented | Unavailable | Unassigned | Not verified |
| TM162 | Bug Buzz | Implemented | Unavailable | Unassigned | Not verified |
| TM164 | Brave Bird | Implemented | Unavailable | Unassigned | Not verified |
| TM165 | Flare Blitz | Implemented | Unavailable | Unassigned | Not verified |
| TM167 | Close Combat | Implemented | Unavailable | Unassigned | Not verified |
| TM169 | Draco Meteor | Implemented | Unavailable | Unassigned | Not verified |
| TM173 | Charge | Implemented | Unavailable | Unassigned | Not verified |
| TM174 | Haze | Implemented | Unavailable | Unassigned | Not verified |
| TM176 | Sand Tomb | Implemented | Unavailable | Unassigned | Not verified |
| TM177 | Spite | Implemented | Available | Hiker Anthony (ID 61), Route 33, x1 | Not verified |
| TM178 | Gravity | Implemented | Unavailable | Unassigned | Not verified |
| TM181 | Knock Off | Implemented | Unavailable | Unassigned | Not verified |
| TM182 | Bug Bite | Implemented | Unavailable | Unassigned | Not verified |
| TM183 | Super Fang | Implemented | Unavailable | Unassigned | Not verified |
| TM184 | Vacuum Wave | Implemented | Unavailable | Unassigned | Not verified |
| TM185 | Lunge | Implemented | Unavailable | Unassigned | Not verified |
| TM186 | High Horsepower | Implemented | Unavailable | Unassigned | Not verified |
| TM187 | Icicle Spear | Implemented | Available | Hiker Phillip (ID 203), Union Cave, x1 | Not verified |
| TM188 | Scald | Implemented | Unavailable | Unassigned | Not verified |
| TM189 | Heat Crash | Implemented | Unavailable | Unassigned | Not verified |
| TM190 | Solar Blade | Implemented | Unavailable | Unassigned | Not verified |
| TM191 | Uproar | Implemented | Unavailable | Unassigned | Not verified |
| TM193 | Weather Ball | Implemented | Unavailable | Unassigned | Not verified |
| TM194 | Grassy Glide | Implemented | Unavailable | Unassigned | Not verified |
| TM196 | Flip Turn | Implemented | Unavailable | Unassigned | Not verified |
| TM197 | Dual Wingbeat | Implemented | Unavailable | Unassigned | Not verified |
| TM198 | Poltergeist | Implemented | Unavailable | Unassigned | Not verified |
| TM199 | Lash Out | Implemented | Unavailable | Unassigned | Not verified |
| TM200 | Scale Shot | Implemented | Unavailable | Unassigned | Not verified |
| TM201 | Misty Explosion | Implemented | Unavailable | Unassigned | Not verified |
| TM202 | Pain Split | Implemented | Available | Firebreather Ray (ID 390), Union Cave, x1 | Not verified |
| TM204 | Double-Edge | Implemented | Unavailable | Unassigned | Not verified |
| TM205 | Endeavor | Implemented | Unavailable | Unassigned | Not verified |
| TM206 | Petal Blizzard | Implemented | Unavailable | Unassigned | Not verified |
| TM207 | Temper Flare | Implemented | Unavailable | Unassigned | Not verified |
| TM208 | Whirlpool | Implemented | Unavailable | Unassigned | Not verified |
| TM209 | Muddy Water | Implemented | Unavailable | Unassigned | Not verified |
| TM210 | Supercell Slam | Implemented | Unavailable | Unassigned | Not verified |
| TM211 | Electroweb | Implemented | Unavailable | Unassigned | Not verified |
| TM212 | Triple Axel | Implemented | Unavailable | Unassigned | Not verified |
| TM213 | Coaching | Implemented | Unavailable | Unassigned | Not verified |
| TM214 | Sludge Wave | Implemented | Unavailable | Unassigned | Not verified |
| TM215 | Scorching Sands | Implemented | Unavailable | Unassigned | Not verified |
| TM216 | Feather Dance | Implemented | Unavailable | Unassigned | Not verified |
| TM217 | Future Sight | Implemented | Unavailable | Unassigned | Not verified |
| TM218 | Expanding Force | Implemented | Unavailable | Unassigned | Not verified |
| TM219 | Skitter Smack | Implemented | Unavailable | Unassigned | Not verified |
| TM220 | Meteor Beam | Implemented | Unavailable | Unassigned | Not verified |
| TM221 | Throat Chop | Implemented | Unavailable | Unassigned | Not verified |
| TM222 | Breaking Swipe | Implemented | Unavailable | Unassigned | Not verified |
| TM223 | Metal Sound | Implemented | Unavailable | Unassigned | Not verified |
| TM224 | Curse | Implemented | Unavailable | Unassigned | Not verified |
| TM228 | Psychic Noise | Implemented | Unavailable | Unassigned | Not verified |

## Unusable TM assignments

This generated table is the explicit do-not-use list. These TM items must not
be assigned as rewards while their moves remain marked
`FLAG_UNUSABLE_UNIMPLEMENTED` in `data/Moves.c`.

<!-- BEGIN GENERATED UNUSABLE TM ASSIGNMENTS -->
| TM item | Move | Engine status | Availability | Reward source | Verification |
| --- | --- | --- | --- | --- | --- |
| TM100 | Confide | Unimplemented | Unusable | Do not assign | N/A |
| TM144 | Fire Pledge | Unimplemented | Unusable | Do not assign | N/A |
| TM145 | Water Pledge | Unimplemented | Unusable | Do not assign | N/A |
| TM146 | Grass Pledge | Unimplemented | Unusable | Do not assign | N/A |
| TM170 | Steel Beam | Unimplemented | Unusable | Do not assign | N/A |
| TM171 | Tera Blast | Unimplemented | Unusable | Do not assign | N/A |
| TM179 | Smack Down | Unimplemented | Unusable | Do not assign | N/A |
| TM195 | Burning Jealousy | Unimplemented | Unusable | Do not assign | N/A |
| TM225 | Hard Press | Unimplemented | Unusable | Do not assign | N/A |
| TM226 | Dragon Cheer | Unimplemented | Unusable | Do not assign | N/A |
| TM227 | Alluring Voice | Unimplemented | Unusable | Do not assign | N/A |
| TM229 | Upper Hand | Unimplemented | Unusable | Do not assign | N/A |
<!-- END GENERATED UNUSABLE TM ASSIGNMENTS -->

TM093, TM094, TM098, TM103, TM105, TM107, TM111, TM114, TM115, TM116,
TM119, TM120, TM125, TM126, TM129, TM135, TM141, TM143, TM148, TM149,
TM150, TM152, TM157, TM158, TM161, TM163, TM166, TM168, TM172, TM175,
TM180, TM192, and TM203 are also omitted from the candidate table because they
repeat moves already taught by TM001-TM092.
