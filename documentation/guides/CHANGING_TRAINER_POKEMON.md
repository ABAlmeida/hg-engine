# Changing Trainer Pokémon

Trainer metadata, parties, AI flags, held items, and battle messages are source
controlled in `data/Trainers.c`. Edit that file rather than an extracted
trainer NARC or a DSPRE-generated copy. `trainerdatagen` converts it into the
trainer data, party, name, and message archives during a ROM build.

## Find the trainer ID

Field scripts start battles using a trainer ID. Locate the script or existing
trainer-table entry and verify the ID before editing it. Do not identify a
trainer solely by display name because multiple battles and rematches can use
the same name with different IDs.

The first Passerby Boy/Silver battle uses three IDs:

| Player's starter | Silver's starter | Trainer ID |
| --- | --- | ---: |
| Chikorita | Cyndaquil | 496 |
| Cyndaquil | Totodile | 497 |
| Totodile | Chikorita | 495 |

Their entries begin at `[495]`, `[496]`, and `[497]` in `data/Trainers.c`.
Change all three when altering Silver 1 so the result does not depend on which
starter the player selected.

## Trainer structure

A trainer entry contains trainer-wide data, a party, and optional battle text:

```c
[495] = {
    .name = "Boy",
    .data = {
        .trainerType = TRAINER_DATA_TYPE_MOVES,
        .trainerClass = TRAINERCLASS_PASSERBY,
        .items = { ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE },
        .aiFlags = F_PRIORITIZE_SUPER_EFFECTIVE |
            F_EVALUATE_ATTACKS |
            F_EXPERT_ATTACKS,
        .battleType = SINGLE_BATTLE,
    },
    .party = {
        {
            .ivs = 255,
            .abilitySlot = TRAINER_POKEMON_ABILITY_1,
            .level = 5,
            .species = SPECIES_CHIKORITA,
            .moves = { MOVE_TACKLE, MOVE_GROWL, MOVE_NONE, MOVE_NONE },
            .ballSeal = 0,
        },
    },
    .text = {
        // Battle messages...
    },
},
```

`trainerdatagen` derives party size from the number of populated `.party`
entries. Do not add or maintain a separate party-count value.

The `.name = "Boy"` and `TRAINERCLASS_PASSERBY` values are appropriate for
Silver 1 while his identity is concealed. Later Silver encounters use their
own trainer IDs and names.

## Change levels or species

Change `.level` and `.species` in the relevant party member:

```c
{
    .ivs = 0,
    .abilitySlot = TRAINER_POKEMON_ABILITY_1,
    .level = 7,
    .species = SPECIES_CHIKORITA,
    .ballSeal = 0,
},
```

For Silver 1, apply the level and common party changes consistently to IDs
495, 496, and 497 while retaining the correct starter in each variant.

## Add or remove party members

Add another initializer inside `.party`:

```c
.party = {
    {
        .ivs = 80,
        .abilitySlot = TRAINER_POKEMON_ABILITY_1,
        .level = 6,
        .species = SPECIES_GASTLY,
        .ballSeal = 0,
    },
    {
        .ivs = 80,
        .abilitySlot = TRAINER_POKEMON_ABILITY_1,
        .level = 7,
        .species = SPECIES_CHIKORITA,
        .ballSeal = 0,
    },
},
```

The source format supports at most six Pokémon per trainer. Party order is the
normal send-out order unless `TRAINER_DATA_RANDOM_PARTY_ORDER` is deliberately
used in the generated party-size field mechanism.

Review messages such as `TRMSG_LAST_POKE_HALF` after changing party order,
because those lines are triggered relative to the trainer's remaining party.

## The legacy IV field

The ordinary `.ivs` field is an 8-bit quality value shared by all six stats.
The runtime scales it from 0-255 to an actual IV from 0-31:

```c
actualIv = sourceValue * 31 / 255;
```

Representative values are:

| Source `.ivs` | Resulting IV in every stat |
| ---: | ---: |
| 0 | 0 |
| 30 | 3 |
| 100 | 12 |
| 200 | 24 |
| 250 | 30 |
| 255 | 31 |

Use this field for a simple uniform IV level. It is not a direct 0-31 field.

## Set separate IVs and EVs

Add `TRAINER_DATA_TYPE_IV_EV_SET` to `.trainerType`:

```c
.trainerType =
    TRAINER_DATA_TYPE_MOVES |
    TRAINER_DATA_TYPE_IV_EV_SET,
```

Then add both structures to every party member in that trainer entry:

```c
.setIvs = {
    .hp = 31,
    .attack = 20,
    .defense = 25,
    .speed = 10,
    .spAttack = 31,
    .spDefense = 25,
},
.setEvs = {
    .hp = 0,
    .attack = 0,
    .defense = 4,
    .speed = 0,
    .spAttack = 252,
    .spDefense = 252,
},
```

Explicit IVs are clamped to 31 by the loader. EVs are stored as bytes and are
not normalized against the usual total limit. For conventionally legal
spreads, keep each EV at or below 252 and the total at or below 510.

The legacy `.ivs` byte remains part of the serialized entry and contributes to
the initial generated Pokémon, but the six explicit IVs replace its stat IVs
before the final stat recalculation.

Explicit IVs and EVs add 12 bytes per configured trainer Pokémon to the
trainer-party NARC. They do not use save space or injected-code memory.

## Custom moves

Without `TRAINER_DATA_TYPE_MOVES`, a trainer Pokémon receives the moves it
would normally know from its species and level.

To choose the moves directly, include the flag:

```c
.trainerType = TRAINER_DATA_TYPE_MOVES,
```

Then populate four positions for every party member:

```c
.moves = {
    MOVE_TACKLE,
    MOVE_GROWL,
    MOVE_RAZOR_LEAF,
    MOVE_NONE,
},
```

Use `MOVE_NONE` for unused positions. When the trainer type includes custom
moves, ensure every member has a complete and intentional move initializer.

## Held items and trainer-used items

Held items require `TRAINER_DATA_TYPE_ITEMS`:

```c
.trainerType =
    TRAINER_DATA_TYPE_MOVES |
    TRAINER_DATA_TYPE_ITEMS,
```

Set the held item on the party member:

```c
.item = ITEM_ORAN_BERRY,
```

This is different from the trainer-wide item array:

```c
.items = { ITEM_POTION, ITEM_NONE, ITEM_NONE, ITEM_NONE },
```

The trainer-wide array contains active battle items available to the enemy
trainer AI. Held items continue to work even when player Bag items are disabled
in trainer battles. Heartless Gold plans to remove active battle-item use from
content, so new trainer-wide items should not be added without revisiting that
decision.

## Abilities, nature, balls, and other options

The normal ability selector supports:

```c
TRAINER_POKEMON_ABILITY_1
TRAINER_POKEMON_ABILITY_2
TRAINER_POKEMON_ABILITY_HIDDEN
```

An exact ability ID, custom Poké Ball, nature, shiny setting, pre-set status,
custom stats, move PP, or nickname requires the corresponding trainer-type or
additional-data flag from `include/trainer_data.h`. Follow an existing working
entry or the loader order in `src/field/enemy_party.c` before enabling one of
these less common fields. The trainer type is a serialized layout bitfield;
adding a field without its flag, or a flag without all required fields, causes
later bytes to be interpreted incorrectly.

## AI flags

The normal decision modules are:

```c
F_PRIORITIZE_SUPER_EFFECTIVE
F_EVALUATE_ATTACKS
F_EXPERT_ATTACKS
F_PRIORITIZE_STATUS_MOVES
F_RISKY_ATTACKS
F_PRIORITIZE_DAMAGE
F_PRIORITIZE_HEALING
F_USE_WEATHER
F_HARRASSMENT
```

`F_HARRASSMENT` is the existing engine spelling. Do not silently correct the
constant name in trainer entries.

Five additional flags represent battle contexts rather than ordinary
difficulty choices:

```c
F_MULTI_BATTLE_PARTNER
F_DOUBLE_BATTLE
F_ROAMING_MON
F_SAFARI_ZONE
F_CATCHING_DEMO
```

Do not add these to an ordinary trainer merely to make its AI harder.

The existing expert combination is:

```c
F_TRAINER_EXPERT_AI
```

It expands to:

```c
F_PRIORITIZE_SUPER_EFFECTIVE |
F_EVALUATE_ATTACKS |
F_EXPERT_ATTACKS
```

This is a sensible default for Gym Leaders, Elite Four members, the Champion,
and important late rival battles. Enabling every strategy flag is not
necessarily stronger: weather, status, risky, damage, healing, and harassment
modules should match the trainer's actual team and intended behavior.

There is currently no separate implemented AI-profile system. Trainer class
does not automatically grant a difficulty profile; each trainer's `.aiFlags`
selects its AI modules. Named source combinations such as `TRAINER_AI_BASIC`,
`TRAINER_AI_ADVANCED`, and `TRAINER_AI_BOSS` could be added later to apply
consistent profiles without changing the serialized format.

## Silver 1 checklist

When changing the first rival battle:

1. Edit trainer IDs 495, 496, and 497.
2. Keep the starter relationship correct for each player choice.
3. Keep common party members, levels, IV policy, moves, and AI consistent
   across all three variants.
4. Retain the Passerby Boy identity unless the story is changed to reveal
   Silver before the battle.
5. Check that the team is appropriate for the current starting level cap.
6. Confirm the scripted battle still selects the intended IDs.
7. Remember that the level-cap increase is keyed by trainer ID, not by the
   species in the party.

## Build and review

From the repository's MSYS2 UCRT64 shell, regenerate and package trainer data
with:

```sh
make quick-rom -j$(nproc)
```

The expected output is `test.nds`. `make code` is insufficient because the
trainer files are packaged NARC data, and `make rebuild_scripts` is unrelated
unless field scripts were also edited.

Review changed trainers for:

- the correct trainer IDs and all required variants;
- no more than six party members;
- valid levels, species, moves, abilities, and items;
- correct trainer-type flags for every serialized optional field;
- legal IV/EV values unless an exception is intentional;
- AI modules appropriate to the team rather than merely numerous;
- messages that still fit the party order and battle type; and
- consistency with level caps, reward mappings, and rematch behavior.
