# Bait Encounters

## Behavior

With `IMPLEMENT_BAIT_ENCOUNTERS` enabled, ordinary step-triggered land and
surfing encounters are suppressed. The original encounter tables remain
intact and continue to supply species, slots, levels, time-of-day variants,
and compatible encounter modifiers.

The option is defined in `include/config.h`. The build derives
`build/armips_config.s` from that definition so the C and Armips portions
cannot be configured independently. Commenting out the C definition restores
passive encounters, the original Repel item metadata and acquisition sources,
and the original Rare Candy rewards.

`Poké Bait` and `Shiny Bait` are new item IDs after the canonical item range.
Both close the Bag and start a dedicated Bait task that generates an encounter
from the current tile. This bypasses the Sweet Scent presentation and hands a
successful result to the normal wild-battle transition:

- Poké Bait starts one ordinary encounter from the current terrain.
- Shiny Bait starts the same encounter and rewrites the generated Pokémon's
  personality to a valid shiny value before it enters the enemy party.
- Successful use consumes one item.
- Invalid terrain is rejected in the Bag with the standard item-use error and
  does not consume the item.
- Bait cannot be used during Safari Zone or Bug-Catching Contest sessions.
- Bait bypasses the roamer branch so it always uses the ordinary encounter
  table.

The Bait-only state is transient and is cleared as soon as the synchronous
encounter-generation call returns. It is not stored in save data, and no
old-save migration is provided.

## Preserved encounters

The passive-encounter patches do not disable roaming, Safari Zone,
Bug-Catching Contest, fishing, Rock Smash, Headbutt, static, or scripted
encounters.

## Item and acquisition changes

- Poké Bait uses a new custom item ID, costs ₽1, and is sold by every ordinary
  Poké Mart as soon as Poké Balls are available. Standalone inventories that
  sell ordinary Poké Balls also include Poké Bait.
- Shiny Bait uses a separate custom item ID and replaces fixed Rare Candy
  pickups and NPC awards.
- Repel, Super Repel, and Max Repel retain their historical numeric IDs for
  table stability but have no field effect and no normal acquisition source.
- Rare Candy retains its original behavior and ID but has no normal
  acquisition source.
- Repeatable Pickup Rare Candy is replaced with PP Up instead of repeatable
  Shiny Bait.
- Poké Bait temporarily reuses the Repel icon. Shiny Bait temporarily reuses
  the Max Repel icon.

Script-archive changes are source-controlled in
`armips/scr_seq/scr_seq_bait_item_rewards.s`. Compiled encounter and item-table
patches are source-controlled in `armips/asm/bait.s`. A normal ROM build
extracts the base data, applies those symbolic patches, and repacks it.

The source-controlled acquisition patch manifest is:

| Source | Offsets | Original reward | Replacement |
| --- | --- | --- | --- |
| ARM9 Mom gift table | `0x10837A` | Repel | Poké Bait |
| Overlay 12 Pickup table | `0x34B4C` | Repel | Poké Bait |
| Overlay 12 Pickup table | `0x34B58` | Rare Candy | PP Up |
| Script member `2_122` | `0x7A1` | Rare Candy | Shiny Bait |
| Script member `2_123` | `0xC10`, `0xCBE` | Rare Candy | Shiny Bait |
| Script member `2_782` | `0x63` | Rare Candy | Shiny Bait |
| Item-ball table `2_141` | `0x46A`, `0x9CE`, `0xABE`, `0xBFE`, `0xD8E`, `0x119E`, `0x128E` | Rare Candy | Shiny Bait |
| Item-ball table `2_141` | `0x64A`, `0xAFA`, `0xF6E` | Repel tier | Poké Bait |

Each acquisition patch checks the existing item ID before replacing it. The
passive land-encounter patches similarly verify that their targets are Thumb
branch-with-link instructions and constrain each replacement to four bytes.
