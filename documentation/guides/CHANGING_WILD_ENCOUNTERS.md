# Changing Wild Encounters

Wild encounter data is source controlled in `data/Encounters.c`. Edit that
file rather than an extracted NARC, generated build file, or DSPRE workspace.
The build converts the table into the encounter NARC at `a/0/3/7`.

Safari Zone encounters are separate and live in `data/SafariEncounters.c`.
Headbutt encounters are also separate and live in `data/Headbutt.c`.

## Find an area's table

Encounter-table identifiers are defined in
`include/constants/encounter_tables.h`. For example, Route 29 uses:

```c
[ENCDATA_R29_ROUTE_29] = {
```

Search `data/Encounters.c` for that identifier and edit only its initializer.
Do not change the numeric position of an entry: maps refer to the generated
NARC member by encounter-table ID.

## Encounter rates

Each table begins with rates for the supported encounter methods:

```c
.rateWalk = 25,
.rateSurf = 0,
.rateRockSmash = 0,
.rateOldRod = 0,
.rateGoodRod = 0,
.rateSuperRod = 0,
```

A zero rate disables that method for the area. These values control how often
the game attempts an encounter; they do not control the relative probability
of the species within that method's slots.

When adding a previously unsupported encounter method, set both a nonzero rate
and valid species/level slots. Do not leave enabled slots filled with
`SPECIES_NONE`.

## Land encounters

Every ordinary land table has:

- one array of 12 levels;
- 12 morning species slots;
- 12 day species slots; and
- 12 night species slots.

The slot weights are fixed by `EncounterSlot_WildMonSlotRoll_Land()` in
`src/field/encounter_check.c`:

| Slot | Probability |
| ---: | ---: |
| 0 | 20% |
| 1 | 20% |
| 2 | 10% |
| 3 | 10% |
| 4 | 10% |
| 5 | 10% |
| 6 | 5% |
| 7 | 5% |
| 8 | 4% |
| 9 | 4% |
| 10 | 1% |
| 11 | 1% |

An area does not need 12 unique species. Repeating a species combines the
probabilities of its slots. A species placed in slots 0 and 1 has a total 40%
probability.

The single level array is shared by all three times of day. For example, the
level at index 4 applies to the morning, day, and night species at index 4.
The current data format cannot assign different morning and night levels to
the same slot.

## Route 29 example

Route 29 starts at `ENCDATA_R29_ROUTE_29` in `data/Encounters.c`. A shortened
example looks like this:

```c
[ENCDATA_R29_ROUTE_29] = {
    .rateWalk = 25,
    .rateSurf = 0,
    .rateRockSmash = 0,
    .rateOldRod = 0,
    .rateGoodRod = 0,
    .rateSuperRod = 0,
    .landSlots = {
        .levels = {
            2, 3, 2, 3, 3, 3, 2, 2, 4, 4, 4, 4
        },
        .speciesMorning = {
            SPECIES_PIDGEY,
            SPECIES_SENTRET,
            // Ten more entries...
        },
        .speciesDay = {
            SPECIES_PIDGEY,
            SPECIES_SENTRET,
            // Ten more entries...
        },
        .speciesNight = {
            SPECIES_HOOTHOOT,
            SPECIES_HOOTHOOT,
            // Ten more entries...
        },
    },
    // Radio, water, fishing, and swarm fields follow.
},
```

Keep exactly 12 entries in each land species array. The compiler may fill an
omitted initializer with zero, but zero is not a valid intentional encounter
species and can produce broken data.

## Make an area independent of time

Use the same 12 species in the morning, day, and night arrays. The simplest
form is to repeat the initializer three times. A local macro can keep one
authoritative list when the three arrays must remain identical:

```c
#define ROUTE_29_LAND_SPECIES \
    {                         \
        SPECIES_PIDGEY,       \
        SPECIES_SENTRET,      \
        SPECIES_PIDGEY,       \
        SPECIES_SENTRET,      \
        SPECIES_RATTATA,      \
        SPECIES_HOPPIP,       \
        SPECIES_SENTRET,      \
        SPECIES_RATTATA,      \
        SPECIES_HOPPIP,       \
        SPECIES_PIDGEY,       \
        SPECIES_RATTATA,      \
        SPECIES_HOPPIP,       \
    }

.landSlots = {
    .levels = {
        2, 3, 2, 3, 3, 3, 2, 2, 4, 4, 4, 4
    },
    .speciesMorning = ROUTE_29_LAND_SPECIES,
    .speciesDay = ROUTE_29_LAND_SPECIES,
    .speciesNight = ROUTE_29_LAND_SPECIES,
},

#undef ROUTE_29_LAND_SPECIES
```

Keep the macro next to the area that uses it and `#undef` it immediately
afterwards. This prevents an area-specific name from leaking into later data.

Do not change the encounter engine to always read the day table merely to make
content time-independent. That would affect every area, leave morning/night
data unused, and risk disagreeing with Pokédex time-of-day information.

## Surfing, fishing, and Rock Smash

These methods store a minimum level, maximum level, and species together:

```c
.surfSlots = {
    { 5, 10, SPECIES_TENTACOOL },
    // Four more entries...
},
```

Their fixed probabilities are:

| Method | Slot probabilities |
| --- | --- |
| Surfing | 60%, 30%, 5%, 4%, 1% |
| Fishing Rod | 40%, 30%, 15%, 10%, 5% |
| Rock Smash | 80%, 20% |

The game selects a level within the slot's inclusive minimum/maximum range.
Use the same number for both values when a slot should have a fixed level.

Heartless Gold presents `ITEM_OLD_ROD` as the Fishing Rod and makes the Good
and Super Rods unobtainable. For an active fishing table, set `rateOldRod` to
90 and populate `oldRodSlots`; leave `rateOldRod` at 0 where fishing is
disabled. `rateGoodRod`, `rateSuperRod`, and their slots remain in the archive
layout for compatibility but are not reachable through normal acquisition.

## Radio and swarm fields

The following fields do not add ordinary slots:

```c
.hoennSoundSpecies = { ... },
.sinnohSoundSpecies = { ... },
.landSwarm = SPECIES_NONE,
.surfSwarm = SPECIES_NONE,
.nightFish = SPECIES_NONE,
.fishSwarm = SPECIES_NONE,
```

They are replacements used when the corresponding Pokégear Radio, swarm, or
fishing condition is active.

For Heartless Gold encounter-table redesigns, the default is that these
conditions must not introduce species outside the area's reviewed list. Point
radio entries and active swarm/night-fishing fields at species already present
in the corresponding ordinary table. Set fields for disabled encounter methods
to `SPECIES_NONE`.

Do not blindly set an active land, surfing, or fishing swarm field to
`SPECIES_NONE`. `src/swarms.c` has a fixed map/type lookup and asserts that the
selected swarm species is valid. If a redesigned area is still present in that
lookup, neutralize its swarm with a species from the ordinary table or make a
separately reviewed change to the lookup.

This is the default content rule, not an engine limitation. A future area may
deliberately restore unique radio, swarm, or night-fishing encounters when its
design explicitly calls for them.

## Keep the Pokédex area display accurate

Actual encounters come from `data/Encounters.c`, but the Pokédex habitat map
comes from `data/PokedexArea.c`. The latter is not generated automatically
from the encounter tables.

When adding or removing a species from an area, audit that species' area lists
in `data/PokedexArea.c` and update its morning, day, and night availability as
needed. A stale Pokédex entry does not change encounters, but it gives the
player incorrect location information.

## Build and review

From the repository's MSYS2 UCRT64 shell, package the changed data with:

```sh
make quick-rom -j$(nproc)
```

The expected output is `test.nds`. `make code` is insufficient because the
change belongs to a packaged data NARC, and `make rebuild_scripts` is unrelated
to encounter tables.

Review each changed area for:

- exactly the required number of slots;
- valid species constants;
- levels appropriate to the current level cap;
- intended aggregate probabilities after repeated slots;
- correct morning/day/night behavior;
- valid data for every enabled encounter method; and
- matching Pokédex habitat information.
