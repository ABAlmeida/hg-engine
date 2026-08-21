# Pokémon Trainers

Last updated: 2026-08-14

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

## Route 30

### Youngster Joey

- First-victory reward: 6 Protein S.
- Party IVs: 25 each.

| Species | Level | Held item |
|---|---:|---|
| Rattata | 9 | None |
| Zubat | 9 | None |
| Pichu | 9 | None |
| Rattata | 10 | Oran Berry |

### Youngster Mikey

- First-victory reward: 6 HP Up S.
- Party IVs: 20 each.

| Species | Level | Held item |
|---|---:|---|
| Zigzagoon | 10 | None |
| Poochyena | 10 | None |
| Bidoof | 11 | Oran Berry |

### Bug Catcher Don

- First-victory reward: 6 Iron S.
- Party IVs: 20 each.

| Species | Level | Held item |
|---|---:|---|
| Butterfree | 10 | Bright Powder |
| Beedrill | 10 | Bright Powder |
| Fomantis | 10 | Bright Powder |
| Sizzlipede | 10 | Bright Powder |

## Route 31

### Bug Catcher Wade

- First-victory reward: 6 Carbos S.
- Party IVs: 25 each.

| Species | Level | Held item |
|---|---:|---|
| Joltik | 11 | None |
| Spinarak | 9 | None |
| Pineco | 9 | None |
| Surskit | 11 | Oran Berry |

## Sprout Tower

### Sage Nico

- First-victory reward: 6 Zinc S.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Growlithe | 9 | None |
| Poliwag | 9 | None |

### Sage Chow

- First-victory reward: 6 Calcium S.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Ralts | 9 | None |
| Shuppet | 9 | None |

### Sage Edmond

- First-victory reward: Full Incense.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Stunky | 9 | None |
| Sandile | 9 | None |

### Sage Jin

- First-victory reward: TM00 (Mega Punch).
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Emolga | 9 | None |
| Foongus | 9 | None |

### Sage Neal

- First-victory reward: Up-Grade.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Klink | 9 | None |
| Tynamo | 9 | None |

### Sage Troy

- First-victory reward: Dubious Disc.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Sandshrew | 9 | None |
| Jigglypuff | 9 | None |

### Sage Li

- First-victory reward: None configured.
- Party IVs: 31 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 11 | None |
| Vulpix | 11 | None |
| Murkrow | 11 | None |
| Croagunk | 11 | None |
| Snover | 11 | Oran Berry |

## Violet Gym

### Bird Keeper Abe

- First-victory reward: Razor Fang.
- Party IVs: 31 each.

| Species | Level | Held item |
|---|---:|---|
| Pidgeotto | 13 | None |

### Bird Keeper Rod

- First-victory reward: 6 Focus Sashes.
- Party IVs: 31 each.

| Species | Level | Held item |
|---|---:|---|
| Pidgeotto | 13 | None |

### Falkner

- First-victory reward: IV Max (custom Gym victory script; not yet manually
  verified).
- Party IVs: 25 each.

| Species | Level | Held item |
|---|---:|---|
| Staravia | 12 | None |
| Skiploom | 12 | None |
| Mantyke | 12 | None |
| Drifblim | 12 | None |
| Noibat | 12 | None |
| Aerodactyl | 13 | Oran Berry |

## Route 32

All parties have 31 IVs in every stat.

| Trainer | Reward | Party (all level 14) |
|---|---|---|
| Youngster Albert | TM095 (Leech Life) | Numel, Corsola, Wobbuffet, Dunsparce |
| Picnicker Liz | 6 HP Up S | Jolteon, Flareon, Vaporeon |
| Camper Roland | 6 Protein S | Baltoy, Blitzle, Bunnelby |
| Fisherman Henry | 6 Iron S | Omastar, Kabutops, Aerodactyl |
| Fisherman Justin | 6 Calcium S | Poliwag, Remoraid, Barboach |
| Fisherman Ralph | 6 Zinc S | Horsea, Starmie, Seel |
| Youngster Gordon | 6 Carbos S | Fletchling, Phanpy, Houndour |
| Bird Keeper Peter | 2 Sitrus Berries | Natu, Delibird, Riolu, Skarmory, Vullaby |

## Union Cave

The 31-IV trainers are Ray (TM202/Pain Split: Charmander, Litwick,
Rotom-Heat), Daniel (Soothe Bell: Geodude, Grimer, Gastly), Russel (Pecha
Berry: Onix, Whismur, Sableye), Bill (Cheri Berry: Magmar, Darumaka, Wooper),
Larry (Chesto Berry: Archen, Hippopotas, Electrike), Leonard (Zinc: Aron,
Aipom, Koffing, Hitmonlee), and Phillip (TM187/Icicle Spear: Rhyhorn, Swinub,
Snover, Minccino, Larvesta). Every listed Pokémon is level 14.

Andrew, Calvin, Nick, Gwen, and Emma each use one level-12 Rattata with 25 IVs
and award one Oran Berry.

## Route 33

- Hiker Anthony: TM177 (Spite); 31 IVs; level-14 Amaura, Hawlucha, Goomy,
  Dedenne, and Klefki.

## Slowpoke Well

| Trainer | IVs | Reward | Party |
|---|---:|---|---|
| Team Rocket Grunt (ID 12) | 31 | Oran Berry | Klink 14, Duskull 14, Hisuian Voltorb 14 |
| Team Rocket Grunt (ID 101) | 31 | Oran Berry | Jigglypuff 14, Bronzor 14, Deino 14 |
| Team Rocket Grunt (ID 13) | 31 | Oran Berry | Furfrou 14, Chansey 14, Audino 14 |
| Executive Proton | 25 | Amulet Coin (custom story script) | Woobat 14, Houndour 15, Skiddo 15, Swirlix 15, Helioptile 16 |

## Azalea Gym

- Al, Benny, and Josh each use one level-16 Heracross with 25 IVs and award
  one Oran Berry.
- Amy & Mimi use two level-16 Heracross with 25 IVs and award one Oran Berry.
- Bugsy uses Heracross 15, Durant 15, Shuckle 15, Shedinja 16, Skorupi 16,
  and Beedrill 17, all with 25 IVs. His custom Gym victory script awards one
  IV Max alongside TM89.
