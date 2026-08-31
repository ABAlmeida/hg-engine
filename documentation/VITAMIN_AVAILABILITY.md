# Vitamin Availability

Last updated: 2026-08-14

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
| HP | HP Up S — Mikey and Liz, 6 each | HP Up — unlimited purchase | HP Up L — unavailable | HP Up Max — unavailable |
| Attack | Protein S — Joey and Roland, 6 each | Protein — unlimited purchase | Protein L — unavailable | Protein Max — unavailable |
| Defense | Iron S — Don and Henry, 6 each | Iron — unlimited purchase | Iron L — unavailable | Iron Max — unavailable |
| Speed | Carbos S — Wade and Gordon, 6 each | Carbos — unlimited purchase | Carbos L — unavailable | Carbos Max — unavailable |
| Sp. Atk | Calcium S — Chow and Justin, 6 each | Calcium — unlimited purchase | Calcium L — unavailable | Calcium Max — unavailable |
| Sp. Def | Zinc S — Nico and Ralph, 6 each | Zinc — unlimited purchase plus Leonard's first-victory reward | Zinc L — unavailable | Zinc Max — unavailable |

The six regular vitamins are sold at both of these locations:

- Goldenrod Department Store 4F.
- Celadon Department Store 5F, right counter.

Because shops do not impose a lifetime purchase limit, every stat has an
unlimited normal-game EV supply once Goldenrod Department Store is accessible.

The S-tier rewards configured in `data/trainer_rewards.csv` are:

| Trainer ID | Trainer | Area | First-victory reward |
|---:|---|---|---|
| 8 | Youngster Joey | Route 30 | 6 Protein S |
| 47 | Youngster Mikey | Route 30 | 6 HP Up S |
| 249 | Bug Catcher Don | Route 30 | 6 Iron S |
| 4 | Bug Catcher Wade | Route 31 | 6 Carbos S |
| 51 | Sage Nico | Sprout Tower | 6 Zinc S |
| 43 | Sage Chow | Sprout Tower | 6 Calcium S |
| 27 | Picnicker Liz | Route 32 | 6 HP Up S |
| 26 | Camper Roland | Route 32 | 6 Protein S |
| 60 | Fisherman Henry | Route 32 | 6 Iron S |
| 18 | Fisherman Justin | Route 32 | 6 Calcium S |
| 57 | Fisherman Ralph | Route 32 | 6 Zinc S |
| 56 | Youngster Gordon | Route 32 | 6 Carbos S |

## IV Max availability

IV Max is not one of the 24 EV vitamins and is therefore excluded from the EV
totals in this document. It has these finite normal-game sources:

| Source | Quantity | Implementation | Verification |
|---|---:|---|---|
| Silver 1 | 1 | Scripted first-victory reward | Existing verified feature |
| Falkner | 1 | Custom Violet Gym victory script | Not verified |
| Bugsy | 1 | Custom Azalea Gym victory script | Not verified |

## Finite normal-game sources

These totals record one-time pickups, gifts, and first-victory rewards. They do
not include shops, debug supplies, or conditional phone gifts.

| Stat | Regular vitamins | S vitamins | Finite nominal EV total | Sources |
|---|---:|---:|---:|---|
| HP | 8 HP Up | 12 HP Up S | 520 | Existing eight regular sources; Mikey and Liz first-victory rewards |
| Attack | 7 Protein | 12 Protein S | 470 | Existing seven regular sources; Joey and Roland first-victory rewards |
| Defense | 6 Iron | 12 Iron S | 420 | Existing six regular sources; Don and Henry first-victory rewards |
| Speed | 6 Carbos | 12 Carbos S | 420 | Existing six regular sources; Wade and Gordon first-victory rewards |
| Sp. Atk | 7 Calcium | 12 Calcium S | 470 | Existing seven regular sources; Chow and Justin first-victory rewards |
| Sp. Def | 4 Zinc | 12 Zinc S | 320 | Existing three hidden Zinc; Leonard's Zinc reward; Nico and Ralph first-victory rewards |

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
| HP | 520 | Unlimited through HP Up | Unlimited |
| Attack | 470 | Unlimited through Protein | Unlimited |
| Defense | 420 | Unlimited through Iron | Unlimited |
| Speed | 420 | Unlimited through Carbos | Unlimited |
| Sp. Atk | 470 | Unlimited through Calcium | Unlimited |
| Sp. Def | 320 | Unlimited through Zinc | Unlimited |

## Debug-cheat availability

`DEBUG_CHEATS` is currently enabled. A new save starts with 10
of every S, regular, L, and Max vitamin in the Medicine pocket. Existing saves
are not seeded again merely by enabling the option.

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
