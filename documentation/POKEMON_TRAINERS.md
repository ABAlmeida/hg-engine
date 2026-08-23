# Pokémon Trainers

Last updated: 2026-08-23

This document tracks the intended trainer parties and first-victory rewards.
Keep it synchronized with `data/Trainers.c`, `data/trainer_rewards.csv`, and
any scripted trainer reward that bypasses the central reward table.

See `guides/CHANGING_TRAINER_POKEMON.md` for the trainer source format and
editing workflow.

## Global trainer rules

- Use `F_TRAINER_EXPERT_AI` for the expert AI profile. This alias replaces the
  former explicit combination of `F_PRIORITIZE_SUPER_EFFECTIVE`,
  `F_EVALUATE_ATTACKS`, and `F_EXPERT_ATTACKS` so future profile changes reach
  every configured expert trainer.
- Trainers use no Bag items during battle.

## New Bark Town

### Silver 1 / Passerby Boy

Trainer IDs 495, 496, and 497 select the starter that is strong against the
player's choice. Every variant uses one level-5 Pokémon with 31 IVs and the
expert AI profile.

| Species | Level | Held item |
|---|---:|---|
| Chikorita, Cyndaquil, or Totodile | 5 | Oran Berry |

## Route 30

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Youngster Joey | 8 | 6 Protein S | 31 | Rattata Lv 11; Zubat Lv 11; Pichu Lv 11; Rattata Lv 12 (Oran Berry) |
| Youngster Mikey | 47 | 6 HP Up S | 31 | Zigzagoon Lv 12; Poochyena Lv 12; Bidoof Lv 13 (Oran Berry) |
| Bug Catcher Don | 249 | 6 Iron S | 31 | Butterfree Lv 11; Beedrill Lv 11; Fomantis Lv 11; Sizzlipede Lv 11 (all hold Bright Powder) |

## Route 31

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bug Catcher Wade | 4 | 6 Carbos S | 31 | Joltik Lv 12; Spinarak Lv 11; Pineco Lv 11; Surskit Lv 12 (Oran Berry) |

## Sprout Tower

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Sage Nico | 51 | 6 Zinc S | 31 | Bellsprout Lv 11; Growlithe Lv 12; Poliwag Lv 13 (Oran Berry) |
| Sage Chow | 43 | 6 Calcium S | 31 | Bellsprout Lv 11; Ralts Lv 12; Shuppet Lv 13 (Oran Berry) |
| Sage Edmond | 52 | Full Incense | 31 | Bellsprout Lv 11; Stunky Lv 12; Sandile Lv 13 (Oran Berry) |
| Sage Jin | 53 | TM00 (Mega Punch) | 31 | Bellsprout Lv 12; Emolga Lv 12; Foongus Lv 13 (Oran Berry) |
| Sage Neal | 55 | Up-Grade | 31 | Bellsprout Lv 13; Klink Lv 13; Tynamo Lv 13 (Oran Berry) |
| Sage Troy | 54 | Dubious Disc | 31 | Bellsprout Lv 13; Sandshrew Lv 13; Jigglypuff Lv 13 (Oran Berry) |
| Sage Li | 290 | None | 31 | Bellsprout Lv 12; Vulpix Lv 12; Murkrow Lv 12; Croagunk Lv 12; Snover Lv 13 (Oran Berry) |

## Violet Gym

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bird Keeper Abe | 50 | Razor Fang | 31 | Ducklett Lv 13; Chatot Lv 13; Venomoth Lv 13; Pidgeotto Lv 13 |
| Bird Keeper Rod | 29 | 6 Focus Sashes | 31 | Archen Lv 13; Woobat Lv 13; Yanma Lv 13; Pidgeotto Lv 13 |
| Falkner | 20 | IV Max (custom victory script) | 25 | Staravia Lv 12; Skiploom Lv 12; Mantyke Lv 12; Drifblim Lv 12; Noibat Lv 12; Aerodactyl Lv 13 (Oran Berry) |

Falkner's IV Max reward remains not manually verified.

## Route 32

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Youngster Albert | 49 | TM095 (Leech Life) | 31 | Numel Lv 15; Corsola Lv 15; Wobbuffet Lv 16; Dunsparce Lv 16 (Oran Berry) |
| Picnicker Liz | 27 | 6 HP Up S | 31 | Jolteon Lv 17; Flareon Lv 17; Vaporeon Lv 17 |
| Camper Roland | 26 | 6 Protein S | 31 | Baltoy Lv 17; Blitzle Lv 17; Bunnelby Lv 17 |
| Fisherman Henry | 60 | 6 Iron S | 31 | Omastar Lv 17; Kabutops Lv 17; Aerodactyl Lv 17 |
| Fisherman Justin | 18 | 6 Calcium S | 31 | Poliwag Lv 17; Remoraid Lv 17; Barboach Lv 17 |
| Fisherman Ralph | 57 | 6 Zinc S | 31 | Horsea Lv 17; Starmie Lv 17; Seel Lv 17 |
| Youngster Gordon | 56 | 6 Carbos S | 31 | Fletchling Lv 17; Phanpy Lv 17; Houndour Lv 17 |
| Bird Keeper Peter | 383 | 2 Sitrus Berries | 31 | Natu Lv 15; Delibird Lv 15; Riolu Lv 16; Skarmory Lv 15 (Oran Berry); Vullaby Lv 16 |

## Union Cave

### 1F

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Firebreather Ray | 390 | TM202 (Pain Split) | 31 | Charmander Lv 17; Litwick Lv 17; Rotom-Heat Lv 17 |
| Hiker Daniel | 384 | Soothe Bell | 31 | Geodude Lv 17; Grimer Lv 17; Gastly Lv 17 |
| Hiker Russel | 25 | Pecha Berry | 31 | Onix Lv 17; Whismur Lv 17; Sableye Lv 17 |
| Firebreather Bill | 319 | Cheri Berry | 31 | Magmar Lv 17; Darumaka Lv 17; Wooper Lv 17 |
| Poké Maniac Larry | 23 | Chesto Berry | 31 | Archen Lv 17; Hippopotas Lv 17; Electrike Lv 17 |

### B1F (North)

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Hiker Leonard | 204 | Zinc | 31 | Aron Lv 16; Aipom Lv 16; Koffing Lv 17; Hitmonlee Lv 17 |
| Hiker Phillip | 203 | TM187 (Icicle Spear) | 31 | Rhyhorn Lv 15; Swinub Lv 15; Snover Lv 15; Minccino Lv 17; Larvesta Lv 16 |

### B1F (South)

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Poké Maniac Andrew | 201 | Oran Berry | 25 | Rattata Lv 12 |
| Poké Maniac Calvin | 202 | Oran Berry | 25 | Rattata Lv 12 |

### B2F

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Ace Trainer Nick | 205 | Oran Berry | 25 | Rattata Lv 12 |
| Ace Trainer Gwen | 206 | Oran Berry | 25 | Rattata Lv 12 |
| Ace Trainer Emma | 363 | Oran Berry | 25 | Rattata Lv 12 |

## Route 33

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Hiker Anthony | 61 | TM177 (Spite) | 31 | Amaura Lv 15; Hawlucha Lv 15; Goomy Lv 16; Dedenne Lv 16; Klefki Lv 17 (Oran Berry) |

## Slowpoke Well

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Team Rocket Grunt 1 | 12 | Oran Berry | 31 | Klink Lv 17; Duskull Lv 17; Hisuian Voltorb Lv 17 |
| Team Rocket Grunt 2 | 101 | Oran Berry | 31 | Jigglypuff Lv 17; Bronzor Lv 17; Deino Lv 17 (Oran Berry) |
| Team Rocket Grunt 3 | 13 | Oran Berry | 31 | Furfrou Lv 17; Chansey Lv 17; Audino Lv 17 (Oran Berry) |
| Executive Proton | 486 | Amulet Coin (custom story script) | 25 | Woobat Lv 15; Houndour Lv 16; Skiddo Lv 16; Swirlix Lv 16; Helioptile Lv 17 (Oran Berry) |

## Azalea Gym

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bug Catcher Al | 68 | Oran Berry | 25 | Butterfree Lv 16; Scyther Lv 16; Volbeat Lv 16; Dwebble Lv 16 |
| Bug Catcher Benny | 67 | Oran Berry | 25 | Parasect Lv 16; Pinsir Lv 16; Beautifly Lv 16; Escavalier Lv 16 |
| Bug Catcher Josh | 69 | Oran Berry | 25 | Ledian Lv 17; Pineco Lv 16; Sizzlipede Lv 16; Accelgor Lv 16 |
| Twins Amy & Mimi | 10 | Oran Berry | 25 | Ariados Lv 16; Ninjask Lv 16; Anorith Lv 16; Dewpider Lv 16 |
| Bugsy | 21 | IV Max (custom victory script) | 25 | Heracross Lv 16; Durant Lv 16; Shuckle Lv 16; Shedinja Lv 16; Skorupi Lv 16; Beedrill Lv 17 |

Bugsy's IV Max reward remains not manually verified.
