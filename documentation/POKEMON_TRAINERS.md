# Pokémon Trainers

Last updated: 2026-08-24

This document tracks the intended trainer parties and first-victory rewards.
Keep it synchronized with `data/Trainers.c`, `data/trainer_rewards.csv`, and
any scripted trainer reward that bypasses the central reward table.

See `guides/CHANGING_TRAINER_POKEMON.md` for the trainer source format and
editing workflow.

Moves are the effective in-game movesets. For trainers without explicit
`.moves` data, this tracker applies the same initialization rules as the game:
it uses implemented level-up moves available at the configured level, ignores
duplicates, and retains the four most recently learned moves.

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

| Species | Level | Held item | Moves |
|---|---:|---|---|
| Chikorita, Cyndaquil, or Totodile | 5 | Oran Berry | Chikorita: Tackle, Growl<br>Cyndaquil: Tackle, Leer, Smokescreen<br>Totodile: Scratch, Leer |

## Route 30

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Youngster Joey | 8 | 6 Protein S | 31 | Rattata Lv 11 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite<br>Zubat Lv 11 — Moves: Absorb, Supersonic, Astonish, Mean Look<br>Pichu Lv 11 — Moves: Tail Whip, Thunder Shock, Play Nice, Sweet Kiss<br>Rattata Lv 12 (Oran Berry) — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |
| Youngster Mikey | 47 | 6 HP Up S | 31 | Zigzagoon Lv 12 — Moves: Sand Attack, Tail Whip, Covet, Headbutt<br>Poochyena Lv 12 — Moves: Tackle, Howl, Sand Attack, Bite<br>Bidoof Lv 13 (Oran Berry) — Moves: Tackle, Defense Curl, Rollout, Headbutt |
| Bug Catcher Don | 249 | 6 Iron S | 31 | Butterfree Lv 11 — Moves: String Shot, Tackle, Supersonic, Confusion<br>Beedrill Lv 11 — Moves: Harden, Poison Sting, String Shot, Fury Cutter<br>Fomantis Lv 11 — Moves: Fury Cutter, Leafage, Growth, Ingrain<br>Sizzlipede Lv 11 (all hold Bright Powder) — Moves: Ember, Smokescreen, Wrap, Bite |

## Route 31

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bug Catcher Wade | 4 | 6 Carbos S | 31 | Joltik Lv 12 — Moves: Fury Cutter, Electroweb, Bug Bite, String Shot<br>Spinarak Lv 11 — Moves: Poison Sting, String Shot, Absorb, Infestation<br>Pineco Lv 11 — Moves: Tackle, Protect, Self-Destruct, Bug Bite<br>Surskit Lv 12 (Oran Berry) — Moves: Water Gun, Quick Attack, Sweet Scent |

## Sprout Tower

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Sage Nico | 51 | 6 Zinc S | 31 | Bellsprout Lv 11 — Moves: Vine Whip, Growth, Wrap<br>Growlithe Lv 12 — Moves: Ember, Howl, Bite, Flame Wheel<br>Poliwag Lv 13 (Oran Berry) — Moves: Water Gun, Hypnosis, Pound, Mud Shot |
| Sage Chow | 43 | 6 Calcium S | 31 | Bellsprout Lv 11 — Moves: Vine Whip, Growth, Wrap<br>Ralts Lv 12 — Moves: Double Team, Confusion, Hypnosis, Draining Kiss<br>Shuppet Lv 13 (Oran Berry) — Moves: Astonish, Screech, Night Shade, Spite |
| Sage Edmond | 52 | Full Incense | 31 | Bellsprout Lv 11 — Moves: Vine Whip, Growth, Wrap<br>Stunky Lv 12 — Moves: Feint, Smokescreen, Acid Spray, Fury Swipes<br>Sandile Lv 13 (Oran Berry) — Moves: Sand Attack, Hone Claws, Sand Tomb, Scary Face |
| Sage Jin | 53 | TM00 (Mega Punch) | 31 | Bellsprout Lv 12 — Moves: Vine Whip, Growth, Wrap<br>Emolga Lv 12 — Moves: Nuzzle, Tail Whip, Double Team, Quick Attack<br>Foongus Lv 13 (Oran Berry) — Moves: Astonish, Growth, Stun Spore, Mega Drain |
| Sage Neal | 55 | Up-Grade | 31 | Bellsprout Lv 13 — Moves: Vine Whip, Growth, Wrap, Sleep Powder<br>Klink Lv 13 — Moves: Vise Grip, Bind, Charge, Charge Beam<br>Tynamo Lv 13 (Oran Berry) — Moves: Tackle, Thunder Wave, Spark, Charge Beam |
| Sage Troy | 54 | Dubious Disc | 31 | Bellsprout Lv 13 — Moves: Vine Whip, Growth, Wrap, Sleep Powder<br>Sandshrew Lv 13 — Moves: Poison Sting, Sand Attack, Rollout, Fury Cutter<br>Jigglypuff Lv 13 (Oran Berry) — Moves: Covet, Stockpile, Spit Up, Swallow |
| Sage Li | 290 | None | 31 | Bellsprout Lv 12 — Moves: Vine Whip, Growth, Wrap<br>Vulpix Lv 12 — Moves: Ember, Disable, Quick Attack, Spite<br>Murkrow Lv 12 — Moves: Peck, Astonish, Gust, Haze<br>Croagunk Lv 12 — Moves: Mud-Slap, Astonish, Taunt, Flatter<br>Snover Lv 13 (Oran Berry) — Moves: Leer, Powder Snow, Leafage, Mist |

## Violet Gym

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bird Keeper Abe | 50 | Razor Fang | 31 | Ducklett Lv 13 — Moves: Water Gun, Defog, Wing Attack, Water Pulse<br>Chatot Lv 13 — Moves: Taunt, Peck, Growl, Sing<br>Venomoth Lv 13 — Moves: Disable, Quiver Dance, Confusion, Poison Powder<br>Pidgeotto Lv 13 — Moves: Gust, Sand Attack, Tackle, Quick Attack |
| Bird Keeper Rod | 29 | 6 Focus Sashes | 31 | Archen Lv 13 — Moves: Rock Throw, Wing Attack, Dragon Breath, Ancient Power<br>Woobat Lv 13 — Moves: Gust, Attract, Confusion, Endeavor<br>Yanma Lv 13 — Moves: Tackle, Quick Attack, Double Team<br>Pidgeotto Lv 13 — Moves: Gust, Sand Attack, Tackle, Quick Attack |
| Falkner | 20 | IV Max (custom victory script) | 25 | Staravia Lv 12 — Moves: Tackle, Growl, Quick Attack, Wing Attack<br>Skiploom Lv 12 — Moves: Fairy Wind, Poison Powder, Stun Spore, Sleep Powder<br>Mantyke Lv 12 — Moves: Water Gun, Supersonic, Wing Attack, Water Pulse<br>Drifblim Lv 12 — Moves: Gust, Focus Energy, Minimize, Payback<br>Noibat Lv 12 — Moves: Absorb, Gust, Supersonic, Double Team<br>Aerodactyl Lv 13 (Oran Berry) — Moves: Ancient Power, Bite, Supersonic, Wing Attack |

Falkner's IV Max reward remains not manually verified.

## Route 32

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Youngster Albert | 49 | TM095 (Leech Life) | 31 | Numel Lv 15 — Moves: Ember, Focus Energy, Bulldoze, Incinerate<br>Corsola Lv 15 — Moves: Tackle, Water Gun, Aqua Ring, Endure<br>Wobbuffet Lv 16 — Moves: Encore, Mirror Coat, Safeguard, Splash<br>Dunsparce Lv 16 (Oran Berry) — Moves: Mud-Slap, Rollout, Glare, Screech |
| Picnicker Liz | 27 | 6 HP Up S | 31 | Jolteon Lv 17 — Moves: Copycat, Sand Attack, Quick Attack, Baby-Doll Eyes<br>Flareon Lv 17 — Moves: Charm, Sand Attack, Quick Attack, Baby-Doll Eyes<br>Vaporeon Lv 17 — Moves: Charm, Sand Attack, Quick Attack, Baby-Doll Eyes |
| Camper Roland | 26 | 6 Protein S | 31 | Baltoy Lv 17 — Moves: Confusion, Rock Tomb, Power Trick, Psybeam<br>Blitzle Lv 17 — Moves: Tail Whip, Charge, Shock Wave, Thunder Wave<br>Bunnelby Lv 17 — Moves: Laser Focus, Quick Attack, Mud Shot, Flail |
| Fisherman Henry | 60 | 6 Iron S | 31 | Omastar Lv 17 — Moves: Rollout, Sand Attack, Withdraw, Water Gun<br>Kabutops Lv 17 — Moves: Sand Attack, Scratch, Slash, Aqua Jet<br>Aerodactyl Lv 17 — Moves: Bite, Supersonic, Wing Attack, Scary Face |
| Fisherman Justin | 18 | 6 Calcium S | 31 | Poliwag Lv 17 — Moves: Water Gun, Hypnosis, Pound, Mud Shot<br>Remoraid Lv 17 — Moves: Water Pulse, Focus Energy, Psybeam, Aurora Beam<br>Barboach Lv 17 — Moves: Mud-Slap, Rest, Snore, Water Pulse |
| Fisherman Ralph | 57 | 6 Zinc S | 31 | Horsea Lv 17 — Moves: Water Gun, Smokescreen, Twister, Focus Energy<br>Starmie Lv 17 — Moves: Surf, Swift, Tackle, Water Gun<br>Seel Lv 17 — Moves: Charm, Icy Wind, Encore, Ice Shard |
| Youngster Gordon | 56 | 6 Carbos S | 31 | Fletchling Lv 17 — Moves: Peck, Quick Attack, Ember, Flail<br>Phanpy Lv 17 — Moves: Defense Curl, Flail, Rollout, Bulldoze<br>Houndour Lv 17 — Moves: Howl, Smog, Roar, Bite |
| Bird Keeper Peter | 383 | 2 Sitrus Berries | 31 | Natu Lv 15 — Moves: Peck, Stored Power, Teleport, Confuse Ray<br>Delibird Lv 15 — Moves: Present<br>Riolu Lv 16 — Moves: Feint, Metal Claw, Counter, Work Up<br>Skarmory Lv 15 (Oran Berry) — Moves: Peck, Sand Attack, Fury Attack, Metal Claw<br>Vullaby Lv 16 — Moves: Gust, Leer, Flatter, Pluck |

## Union Cave

### 1F

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Firebreather Ray | 390 | TM202 (Pain Split) | 31 | Charmander Lv 17 — Moves: Ember, Smokescreen, Dragon Breath, Fire Fang<br>Litwick Lv 17 — Moves: Ember, Minimize, Confuse Ray, Hex<br>Rotom-Heat Lv 17 — Moves: Astonish, Thunder Shock, Confuse Ray, Charge |
| Hiker Daniel | 384 | Soothe Bell | 31 | Geodude Lv 17 — Moves: Rock Polish, Rollout, Bulldoze, Rock Throw<br>Grimer Lv 17 — Moves: Harden, Mud-Slap, Disable, Sludge<br>Gastly Lv 17 — Moves: Hypnosis, Mean Look, Payback, Spite |
| Hiker Russel | 25 | Pecha Berry | 31 | Onix Lv 17 — Moves: Tackle, Rock Polish, Dragon Breath, Curse<br>Whismur Lv 17 — Moves: Pound, Howl, Rest, Sleep Talk<br>Sableye Lv 17 — Moves: Astonish, Shadow Sneak, Fake Out, Disable |
| Firebreather Bill | 319 | Cheri Berry | 31 | Magmar Lv 17 — Moves: Smokescreen, Smog, Clear Smog, Flame Wheel<br>Darumaka Lv 17 — Moves: Taunt, Bite, Incinerate, Work Up<br>Wooper Lv 17 — Moves: Mud Shot, Mist, Haze, Slam |
| Poké Maniac Larry | 23 | Chesto Berry | 31 | Archen Lv 17 — Moves: Wing Attack, Dragon Breath, Ancient Power, Pluck<br>Hippopotas Lv 17 — Moves: Bite, Yawn, Sand Tomb, Dig<br>Electrike Lv 17 — Moves: Leer, Howl, Quick Attack, Shock Wave |

### B1F (North)

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Hiker Leonard | 204 | Zinc | 31 | Aron Lv 16 — Moves: Metal Claw, Rock Tomb, Roar, Headbutt<br>Aipom Lv 16 — Moves: Sand Attack, Astonish, Baton Pass, Tickle<br>Koffing Lv 17 — Moves: Smog, Smokescreen, Clear Smog, Assurance<br>Hitmonlee Lv 17 — Moves: Double Kick, Low Kick, Endure, Sucker Punch |
| Hiker Phillip | 203 | TM187 (Icicle Spear) | 31 | Rhyhorn Lv 15 — Moves: Tackle, Tail Whip, Bulldoze, Horn Attack<br>Swinub Lv 15 — Moves: Mud-Slap, Powder Snow, Flail, Ice Shard<br>Snover Lv 15 — Moves: Powder Snow, Leafage, Mist, Ice Shard<br>Minccino Lv 17 — Moves: Baby-Doll Eyes, Helping Hand, Sing, Charm<br>Larvesta Lv 16 — Moves: Ember, String Shot, Flame Charge, Struggle Bug |

### B1F (South)

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Poké Maniac Andrew | 201 | Oran Berry | 25 | Rattata Lv 12 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |
| Poké Maniac Calvin | 202 | Oran Berry | 25 | Rattata Lv 12 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |

### B2F

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Ace Trainer Nick | 205 | Oran Berry | 25 | Rattata Lv 12 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |
| Ace Trainer Gwen | 206 | Oran Berry | 25 | Rattata Lv 12 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |
| Ace Trainer Emma | 363 | Oran Berry | 25 | Rattata Lv 12 — Moves: Tail Whip, Quick Attack, Focus Energy, Bite |

## Route 33

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Hiker Anthony | 61 | TM177 (Spite) | 31 | Amaura Lv 15 — Moves: Growl, Encore, Ancient Power, Icy Wind<br>Hawlucha Lv 15 — Moves: Hone Claws, Wing Attack, Detect, Aerial Ace<br>Goomy Lv 16 — Moves: Absorb, Water Gun, Dragon Breath, Protect<br>Dedenne Lv 16 — Moves: Nuzzle, Tackle, Charge, Thunder Shock<br>Klefki Lv 17 (Oran Berry) — Moves: Astonish, Tackle, Fairy Wind, Torment |

## Slowpoke Well

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Team Rocket Grunt 1 | 12 | Oran Berry | 31 | Klink Lv 17 — Moves: Bind, Charge, Charge Beam, Metal Sound<br>Duskull Lv 17 — Moves: Disable, Shadow Sneak, Confuse Ray, Night Shade<br>Hisuian Voltorb Lv 17 — Moves: Bullet Seed, Rollout, Screech, Charge Beam |
| Team Rocket Grunt 2 | 101 | Oran Berry | 31 | Jigglypuff Lv 17 — Moves: Covet, Stockpile, Spit Up, Swallow<br>Bronzor Lv 17 — Moves: Confuse Ray, Payback, Imprison, Gyro Ball<br>Deino Lv 17 (Oran Berry) — Moves: Dragon Breath, Bite, Roar, Assurance |
| Team Rocket Grunt 3 | 13 | Oran Berry | 31 | Furfrou Lv 17 — Moves: Sand Attack, Baby-Doll Eyes, Headbutt, Tail Whip<br>Chansey Lv 17 — Moves: Defense Curl, Tail Whip, Life Dew, Sing<br>Audino Lv 17 (Oran Berry) — Moves: Disarming Voice, Baby-Doll Eyes, Helping Hand, Growl |
| Executive Proton | 486 | Amulet Coin (custom story script) | 25 | Woobat Lv 15 — Moves: Attract, Confusion, Endeavor, Air Cutter<br>Houndour Lv 16 — Moves: Howl, Smog, Roar, Bite<br>Skiddo Lv 16 — Moves: Tail Whip, Leech Seed, Razor Leaf, Worry Seed<br>Swirlix Lv 16 — Moves: Fairy Wind, Aromatherapy, Draining Kiss, Fake Tears<br>Helioptile Lv 17 (Oran Berry) — Moves: Pound, Thunder Shock, Quick Attack, Charge |

## Azalea Gym

| Trainer | ID | Reward | IVs | Party |
|---|---:|---|---:|---|
| Bug Catcher Al | 68 | Oran Berry | 25 | Butterfree Lv 16 — Moves: Poison Powder, Sleep Powder, Stun Spore, Psybeam<br>Scyther Lv 16 — Moves: Fury Cutter, False Swipe, Wing Attack, Double Team<br>Volbeat Lv 16 — Moves: Double Team, Confuse Ray, Quick Attack, Struggle Bug<br>Dwebble Lv 16 — Moves: Sand Attack, Withdraw, Bug Bite, Flail |
| Bug Catcher Benny | 67 | Oran Berry | 25 | Parasect Lv 16 — Moves: Poison Powder, Scratch, Stun Spore, Absorb<br>Pinsir Lv 16 — Moves: Focus Energy, Bind, Seismic Toss, Bug Bite<br>Beautifly Lv 16 — Moves: String Shot, Tackle, Absorb, Stun Spore<br>Escavalier Lv 16 — Moves: Fury Cutter, Quick Guard, False Swipe, Acid Spray |
| Bug Catcher Josh | 69 | Oran Berry | 25 | Ledian Lv 17 — Moves: Light Screen, Reflect, Safeguard, Mach Punch<br>Pineco Lv 16 — Moves: Protect, Self-Destruct, Bug Bite, Take Down<br>Sizzlipede Lv 16 — Moves: Smokescreen, Wrap, Bite, Flame Wheel<br>Accelgor Lv 16 — Moves: Acid Spray, Quick Attack, Mega Drain, Struggle Bug |
| Twins Amy & Mimi | 10 | Oran Berry | 25 | Ariados Lv 16 — Moves: Focus Energy, Infestation, Scary Face, Night Shade<br>Ninjask Lv 16 — Moves: Sand Attack, Scratch, Screech, Agility<br>Anorith Lv 16 — Moves: Harden, Water Gun, Metal Claw, Ancient Power<br>Dewpider Lv 16 — Moves: Bug Bite, Bite, Bubble Beam, Aqua Ring |
| Bugsy | 21 | IV Max (custom victory script) | 25 | Heracross Lv 16 — Moves: Arm Thrust, Fury Attack, Endure, Aerial Ace<br>Durant Lv 16 — Moves: Vise Grip, Metal Claw, Beat Up, Bug Bite<br>Shuckle Lv 16 — Moves: Wrap, Rollout, Struggle Bug, Rock Throw<br>Shedinja Lv 16 — Moves: Sand Attack, Scratch, Shadow Claw, Confuse Ray<br>Skorupi Lv 16 — Moves: Fell Stinger, Poison Fang, Bite, Toxic Spikes<br>Beedrill Lv 17 — Moves: Poison Sting, String Shot, Fury Cutter, Laser Focus |

Bugsy's IV Max reward remains not manually verified.
