# Heartless Gold Level Caps

Heartless Gold stores the current level cap in the expanded miscellaneous save
block. New saves start at level 5. Winning a configured story battle calls
`SetNewLevelCap`; the setter accepts equal or higher caps and ignores lower
caps.

The cap is awarded by the common trainer-battle completion hook. Individual
event scripts do not set or calculate it.

## Progression

| Victory | New cap |
| --- | ---: |
| New save | 5 |
| Silver fight 1 | 13 |
| Falkner | 17 |
| Bugsy | 18 |
| Silver fight 2 | 19 |
| Whitney | 22 |
| Silver fight 3 | 25 |
| Morty | 31 |
| Chuck | 35 |
| Jasmine | 35 |
| Pryce | 35 |
| Silver fight 4 | 38 |
| Executive Archer | 41 |
| Clair | 41 |
| Silver fight 5 | 42 |
| Will | 44 |
| Koga | 46 |
| Bruno | 47 |
| Karen | 50 |
| Lance | 55 |
| Lt. Surge | 55 |
| Sabrina | 55 |
| Erika | 55 |
| Janine | 55 |
| Misty | 55 |
| Brock | 55 |
| Silver fight 6 | 59 |
| Blaine | 60 |
| Blue | 86 |
| Red | 100 |

Every listed milestone has an independent reward-table entry even when its
current cap is unchanged. This lets future balancing change one fight without
altering the battle hook or progression code.

## Silver trainer IDs

Each Silver battle has three trainer records for his starter-dependent team.

| Battle | Trainer IDs |
| --- | --- |
| Silver 1 | 495, 496, 497 |
| Silver 2 | 1, 266, 269 |
| Silver 3 | 263, 267, 270 |
| Silver 4 | 271, 288, 289 |
| Silver 5 | 264, 268, 272 |
| Silver 6 | 285, 286, 287 |

Silver fight 1 uses the `Boy`/Passerby trainer records because it happens
before the rival is named. The later fights use the `Silver`/Rival records.

The complete symbolic trainer IDs are defined in
`include/constants/trainer_id.h`. The cap reward table is in
`src/level_cap.c`.

## Save behavior

- `SAVE_MISC_DATA.levelCap` is initialized to 5 for a new save.
- A configured victory writes its cap before control returns to the field.
- A reward lower than the saved cap is ignored.
- An equal reward is accepted, so same-cap milestones remain explicit.
- Normal saving persists the field with the rest of `SAVE_MISC_DATA`.
- Ordinary HeartGold saves and earlier development saves are not migrated.

## Battle exclusions

Only a player victory in a normal trainer battle can apply a reward. Wireless,
Battle Tower, automated debug, wild, and unrelated trainer battles do not
change the cap. Rematch trainer IDs are not included in the reward table.
