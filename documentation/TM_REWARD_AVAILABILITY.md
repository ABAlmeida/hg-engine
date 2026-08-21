# TM Reward Availability

Last updated: 2026-08-13

This document tracks the distinct, implemented TM moves that are not currently
obtainable as TMs in Heartless Gold. It is intended to support assigning TMs as
first-victory trainer rewards.

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

## Updating this tracker

When adding a TM to `data/trainer_rewards.csv`:

1. Change **Availability** from `Unavailable` to `Available`.
2. Record the trainer name, numeric trainer ID, and location under **Reward source**.
3. Leave **Verification** as `Not verified` until the reward has been received
   and the TM has been used successfully in a current build.
4. After that manual check, change **Verification** to `Verified` and update the
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

## Excluded TM assignments

The following TM assignments are not reward candidates because their moves are
explicitly marked unimplemented:

| TM item | Move |
|---|---|
| TM100 | Confide |
| TM144 | Fire Pledge |
| TM145 | Water Pledge |
| TM146 | Grass Pledge |
| TM170 | Steel Beam |
| TM171 | Tera Blast |
| TM179 | Smack Down |
| TM195 | Burning Jealousy |
| TM225 | Hard Press |
| TM226 | Dragon Cheer |
| TM227 | Alluring Voice |
| TM229 | Upper Hand |

TM093, TM094, TM098, TM103, TM105, TM107, TM111, TM114, TM115, TM116,
TM119, TM120, TM125, TM126, TM129, TM135, TM141, TM143, TM148, TM149,
TM150, TM152, TM157, TM158, TM161, TM163, TM166, TM168, TM172, TM175,
TM180, TM192, and TM203 are also omitted from the candidate table because they
repeat moves already taught by TM001-TM092.
