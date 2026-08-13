# Vitamin Availability

Last updated: 2026-08-13

This document tracks all 24 EV-training vitamin items, their normal gameplay
sources, and the amount of EV training available for each stat.

Availability is tracked separately from item behavior. All four tiers are
implemented for all six stats in `src/stat_training_items.c`, but most custom
tiers have not yet been placed in normal gameplay. Debug-cheat supplies are
listed separately and do not count as intended progression availability.
The source audit covers the inherited HGSS event tables plus Heartless Gold's
source-controlled mart and trainer-reward overrides.

## EV amounts

| Tier | EVs added by one item |
|---|---:|
| S | 10 |
| Regular/unsuffixed | 50 |
| L | 100 |
| Max | As many as fit, up to 252 in the selected stat |

Every tier respects the 252-per-stat and 510-total EV limits. The actual gain
can therefore be lower than the nominal amount.

## Normal gameplay availability

| Stat | S vitamin | Regular vitamin | L vitamin | Max vitamin |
|---|---|---|---|---|
| HP | HP Up S — Youngster Mikey, Route 30, first victory | HP Up — unlimited purchase | HP Up L — unavailable | HP Up Max — unavailable |
| Attack | Protein S — Youngster Joey, Route 30, first victory | Protein — unlimited purchase | Protein L — unavailable | Protein Max — unavailable |
| Defense | Iron S — Bug Catcher Don, Route 30, first victory | Iron — unlimited purchase | Iron L — unavailable | Iron Max — unavailable |
| Speed | Carbos S — Bug Catcher Wade, Route 31, first victory | Carbos — unlimited purchase | Carbos L — unavailable | Carbos Max — unavailable |
| Sp. Atk | Calcium S — Sage Chow, Sprout Tower, first victory | Calcium — unlimited purchase | Calcium L — unavailable | Calcium Max — unavailable |
| Sp. Def | Zinc S — Sage Nico, Sprout Tower, first victory | Zinc — unlimited purchase | Zinc L — unavailable | Zinc Max — unavailable |

The six regular vitamins are sold at both of these locations:

- Goldenrod Department Store 4F.
- Celadon Department Store 5F, right counter.

Because shops do not impose a lifetime purchase limit, every stat has an
unlimited normal-game EV supply once Goldenrod Department Store is accessible.

The S-tier rewards configured in `data/trainer_rewards.csv` are:

| Trainer ID | Trainer | Area | First-victory reward |
|---:|---|---|---|
| 8 | Youngster Joey | Route 30 | Protein S |
| 47 | Youngster Mikey | Route 30 | HP Up S |
| 249 | Bug Catcher Don | Route 30 | Iron S |
| 4 | Bug Catcher Wade | Route 31 | Carbos S |
| 51 | Sage Nico | Sprout Tower | Zinc S |
| 43 | Sage Chow | Sprout Tower | Calcium S |

## Finite normal-game sources

These totals record one-time pickups, gifts, and first-victory rewards. They do
not include shops, debug supplies, or conditional phone gifts.

| Stat | Regular vitamins | S vitamins | Finite nominal EV total | Sources |
|---|---:|---:|---:|---|
| HP | 8 HP Up | 1 HP Up S | 410 | Burned Tower 1F; Route 4; Victory Road 2F; Bell Tower 9F; hidden on Route 38, Mt. Mortar 2F, and Rock Tunnel B1F; Route 35 Goldenrod Gatehouse Spearow-mail reward; Youngster Mikey first-victory reward |
| Attack | 7 Protein | 1 Protein S | 360 | Route 25; Ice Path 1F; Team Rocket HQ B3F; Mt. Silver Cave 2F; hidden in the Pokéathlon Dome, Olivine Port exterior, and Cerulean Cave 2F; Youngster Joey first-victory reward |
| Defense | 6 Iron | 1 Iron S | 310 | Ice Path B1F; Mt. Mortar 1F back area; Rock Tunnel B1F; hidden at Vermilion Port exterior, Cinnabar Island, and Mt. Silver Cave 2F; Bug Catcher Don first-victory reward |
| Speed | 6 Carbos | 1 Carbos S | 310 | Whirl Islands B1F; Mt. Mortar B1F; Route 2 east; hidden in Bell Tower 8F, Ice Path B2F, and Route 10; Bug Catcher Wade first-victory reward |
| Sp. Atk | 7 Calcium | 1 Calcium S | 360 | Route 12; Whirl Islands B1F; Dragon's Den; Mt. Silver Cave 2F; hidden on Route 13, Union Cave B2F, and Diglett's Cave; Sage Chow first-victory reward |
| Sp. Def | 3 Zinc | 1 Zinc S | 160 | Hidden in Cerulean Cave 2F, Victory Road 3F, and Seafoam Islands B4F; Sage Nico first-victory reward |

The finite nominal total assumes every item can apply its full amount across
eligible Pokémon. Per-Pokémon EV caps can reduce the actual amount gained.

## Conditional phone gifts

The inherited HGSS phone data also names these regular-vitamin gifts. They are
not included in the finite totals because their delivery depends on the phone
and rematch flow:

| Stat | Vitamin | Contact | Location |
|---|---|---|---|
| HP | HP Up | Youngster Joey | Route 30 |
| Attack | Protein | Sailor Huey | Olivine Lighthouse 2F |
| Defense | Iron | Hiker Parry | Route 45 |
| Speed | Carbos | Bird Keeper Vance | Route 44 |
| Sp. Atk | Calcium | Picnicker Erin | Route 46 |
| Sp. Def | — | No vitamin phone gift found | — |

## Total EV availability by stat

| Stat | Finite one-time EVs | Purchasable EVs | Total normal-game availability |
|---|---:|---|---|
| HP | 410 | Unlimited through HP Up | Unlimited |
| Attack | 360 | Unlimited through Protein | Unlimited |
| Defense | 310 | Unlimited through Iron | Unlimited |
| Speed | 310 | Unlimited through Carbos | Unlimited |
| Sp. Atk | 360 | Unlimited through Calcium | Unlimited |
| Sp. Def | 160 | Unlimited through Zinc | Unlimited |

## Debug-cheat availability

`DEBUG_CHEATS` is currently enabled. On a new save, the Medicine pocket starts
with 10 of every S, regular, L, and Max vitamin. Existing saves are not seeded
again merely by enabling the option.

For each stat, the seeded items represent a nominal maximum of 4,120 EVs across
eligible Pokémon:

- 10 S vitamins: 100 EVs.
- 10 regular vitamins: 500 EVs.
- 10 L vitamins: 1,000 EVs.
- 10 Max vitamins: up to 2,520 EVs.

This debug quantity is excluded from all normal-game totals above.

## Updating this tracker

When adding or removing a vitamin source:

1. Update the relevant item in **Normal gameplay availability**.
2. Record the exact trainer, shop, pickup, gift, or prize under **Finite
   normal-game sources** when the source has a lifetime quantity limit.
3. Recalculate the finite nominal total using the tier amounts above.
4. Do not replace `Unlimited` with a finite number while any unlimited shop or
   repeatable source remains.
5. Record manual source verification separately from the already verified item
   behavior; defining an item or reward row does not prove it can be obtained.
