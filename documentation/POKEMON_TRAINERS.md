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

- First-victory reward: Protein S.
- Party IVs: 25 each.

| Species | Level | Held item |
|---|---:|---|
| Rattata | 9 | None |
| Zubat | 9 | None |
| Pichu | 9 | None |
| Rattata | 10 | Oran Berry |

### Youngster Mikey

- First-victory reward: HP Up S.
- Party IVs: 20 each.

| Species | Level | Held item |
|---|---:|---|
| Zigzagoon | 10 | None |
| Poochyena | 10 | None |
| Bidoof | 11 | Oran Berry |

### Bug Catcher Don

- First-victory reward: Iron S.
- Party IVs: 20 each.

| Species | Level | Held item |
|---|---:|---|
| Butterfree | 10 | Bright Powder |
| Beedrill | 10 | Bright Powder |
| Fomantis | 10 | Bright Powder |
| Sizzlipede | 10 | Bright Powder |

## Route 31

### Bug Catcher Wade

- First-victory reward: Carbos S.
- Party IVs: 25 each.

| Species | Level | Held item |
|---|---:|---|
| Joltik | 11 | None |
| Spinarak | 9 | None |
| Pineco | 9 | None |
| Surskit | 11 | Oran Berry |

## Sprout Tower

### Sage Nico

- First-victory reward: Zinc S.
- Party IVs: 15 each.

| Species | Level | Held item |
|---|---:|---|
| Bellsprout | 9 | None |
| Growlithe | 9 | None |
| Poliwag | 9 | None |

### Sage Chow

- First-victory reward: Calcium S.
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

- First-victory reward: Lax Incense.
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

- First-victory reward: Blunder Policy.
- Party IVs: 31 each.

| Species | Level | Held item |
|---|---:|---|
| Pidgeotto | 13 | None |

### Bird Keeper Rod

- First-victory reward: Razor Fang.
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
