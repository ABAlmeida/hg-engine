# Held-Item Reward Availability

Last updated: 2026-08-25

This document tracks non-Berry held items as candidates for first-victory
trainer rewards. A candidate is an item with a nonzero implemented
`holdEffect` in `data/itemdata/itemdata.c`; Berries are tracked separately in
`BERRY_REWARD_AVAILABILITY.md`.

This is a reward-placement tracker, not yet a complete audit of every pickup,
shop, gift, wild held item, or competitive interaction. **Other normal source**
therefore remains `Not audited` until that item's wider availability is
checked deliberately.

## Status summary

| Status | Count |
|---|---:|
| Non-Berry held-item candidates | 202 |
| Assigned as trainer rewards | 10 |
| Manually verified trainer rewards | 0 |

## Updating this tracker

When adding a non-Berry held item to `data/trainer_rewards.csv`:

1. Change **Trainer reward** from `Unassigned` to the trainer name, numeric
   trainer ID, location, and quantity.
2. Leave **Verification** as `Not verified` until the reward has been received
   successfully in a current build.
3. After that manual check, change **Verification** to `Verified` and update
   the summary counts above.
4. Audit and update **Other normal source** only when the task explicitly
   includes broader item availability.

Some candidates are species-specific, form-changing, evolutionary, obsolete
under current rules, or unsuitable for early progression. Inclusion confirms
an implemented held effect, not that the item is an appropriate reward.

## Held-item candidates

| Item ID | Held item | Item constant | Hold effect | Other normal source | Trainer reward | Verification |
|---:|---|---|---|---|---|---|
| 43 | Berry Juice | `ITEM_BERRY_JUICE` | `HOLD_EFFECT_HP_RESTORE` | Not audited | Unassigned | Not verified |
| 112 | Griseous Orb | `ITEM_GRISEOUS_ORB` | `HOLD_EFFECT_GIRATINA_BOOST` | Not audited | Unassigned | Not verified |
| 116 | Douse Drive | `ITEM_DOUSE_DRIVE` | `HOLD_EFFECT_DOUSE_DRIVE` | Not audited | Unassigned | Not verified |
| 117 | Shock Drive | `ITEM_SHOCK_DRIVE` | `HOLD_EFFECT_SHOCK_DRIVE` | Not audited | Unassigned | Not verified |
| 118 | Burn Drive | `ITEM_BURN_DRIVE` | `HOLD_EFFECT_BURN_DRIVE` | Not audited | Unassigned | Not verified |
| 119 | Chill Drive | `ITEM_CHILL_DRIVE` | `HOLD_EFFECT_CHILL_DRIVE` | Not audited | Unassigned | Not verified |
| 135 | Adamant Orb | `ITEM_ADAMANT_ORB` | `HOLD_EFFECT_DIALGA_BOOST` | Not audited | Unassigned | Not verified |
| 136 | Lustrous Orb | `ITEM_LUSTROUS_ORB` | `HOLD_EFFECT_PALKIA_BOOST` | Not audited | Unassigned | Not verified |
| 213 | Bright Powder | `ITEM_BRIGHT_POWDER` | `HOLD_EFFECT_ACC_REDUCE` | Not audited | Unassigned | Not verified |
| 214 | White Herb | `ITEM_WHITE_HERB` | `HOLD_EFFECT_STATDOWN_RESTORE` | Not audited | Unassigned | Not verified |
| 215 | Macho Brace | `ITEM_MACHO_BRACE` | `HOLD_EFFECT_EVS_UP_SPEED_DOWN` | Not audited | Unassigned | Not verified |
| 216 | Exp. Share | `ITEM_EXP_SHARE` | `HOLD_EFFECT_EXP_SHARE` | Not audited | Unassigned | Not verified |
| 217 | Quick Claw | `ITEM_QUICK_CLAW` | `HOLD_EFFECT_SOMETIMES_PRIORITY` | Not audited | Unassigned | Not verified |
| 218 | Soothe Bell | `ITEM_SOOTHE_BELL` | `HOLD_EFFECT_FRIENDSHIP_UP` | Not audited | Hiker Daniel (ID 384), Union Cave, x1 | Not verified |
| 219 | Mental Herb | `ITEM_MENTAL_HERB` | `HOLD_EFFECT_HEAL_MENTAL_CONDITIONS` | Not audited | Unassigned | Not verified |
| 220 | Choice Band | `ITEM_CHOICE_BAND` | `HOLD_EFFECT_CHOICE_ATK` | Not audited | Unassigned | Not verified |
| 221 | King's Rock | `ITEM_KINGS_ROCK` | `HOLD_EFFECT_SOMETIMES_FLINCH` | Not audited | Unassigned | Not verified |
| 222 | Silver Powder | `ITEM_SILVER_POWDER` | `HOLD_EFFECT_STRENGTHEN_BUG` | Not audited | Unassigned | Not verified |
| 223 | Amulet Coin | `ITEM_AMULET_COIN` | `HOLD_EFFECT_MONEY_UP` | Not audited | Executive Proton (ID 486), Slowpoke Well, x1 scripted | Not verified |
| 224 | Cleanse Tag | `ITEM_CLEANSE_TAG` | `HOLD_EFFECT_ENCOUNTERS_DOWN` | Not audited | Unassigned | Not verified |
| 225 | Soul Dew | `ITEM_SOUL_DEW` | `HOLD_EFFECT_LATI_SPECIAL` | Not audited | Unassigned | Not verified |
| 226 | Deep Sea Tooth | `ITEM_DEEP_SEA_TOOTH` | `HOLD_EFFECT_CLAMPERL_SPATK` | Not audited | Team Rocket Grunt 2 (ID 101), Slowpoke Well, x1 | Not verified |
| 227 | Deep Sea Scale | `ITEM_DEEP_SEA_SCALE` | `HOLD_EFFECT_CLAMPERL_SPDEF` | Not audited | Team Rocket Grunt 3 (ID 13), Slowpoke Well, x1 | Not verified |
| 228 | Smoke Ball | `ITEM_SMOKE_BALL` | `HOLD_EFFECT_FLEE` | Not audited | Unassigned | Not verified |
| 229 | Everstone | `ITEM_EVERSTONE` | `HOLD_EFFECT_NO_EVOLVE` | Not audited | Unassigned | Not verified |
| 230 | Focus Band | `ITEM_FOCUS_BAND` | `HOLD_EFFECT_MAYBE_ENDURE` | Not audited | Unassigned | Not verified |
| 231 | Lucky Egg | `ITEM_LUCKY_EGG` | `HOLD_EFFECT_EXP_UP` | Not audited | Unassigned | Not verified |
| 232 | Scope Lens | `ITEM_SCOPE_LENS` | `HOLD_EFFECT_CRITRATE_UP` | Not audited | Unassigned | Not verified |
| 233 | Metal Coat | `ITEM_METAL_COAT` | `HOLD_EFFECT_STRENGTHEN_STEEL` | Not audited | Unassigned | Not verified |
| 234 | Leftovers | `ITEM_LEFTOVERS` | `HOLD_EFFECT_HP_RESTORE_GRADUAL` | Not audited | Unassigned | Not verified |
| 235 | Dragon Scale | `ITEM_DRAGON_SCALE` | `HOLD_EFFECT_EVOLVE_SEADRA` | Not audited | Unassigned | Not verified |
| 236 | Light Ball | `ITEM_LIGHT_BALL` | `HOLD_EFFECT_PIKA_SPATK_UP` | Not audited | Unassigned | Not verified |
| 237 | Soft Sand | `ITEM_SOFT_SAND` | `HOLD_EFFECT_STRENGTHEN_GROUND` | Not audited | Unassigned | Not verified |
| 238 | Hard Stone | `ITEM_HARD_STONE` | `HOLD_EFFECT_STRENGTHEN_ROCK` | Not audited | Unassigned | Not verified |
| 239 | Miracle Seed | `ITEM_MIRACLE_SEED` | `HOLD_EFFECT_STRENGTHEN_GRASS` | Not audited | Unassigned | Not verified |
| 240 | Black Glasses | `ITEM_BLACK_GLASSES` | `HOLD_EFFECT_STRENGTHEN_DARK` | Not audited | Unassigned | Not verified |
| 241 | Black Belt | `ITEM_BLACK_BELT` | `HOLD_EFFECT_STRENGTHEN_FIGHT` | Not audited | Unassigned | Not verified |
| 242 | Magnet | `ITEM_MAGNET` | `HOLD_EFFECT_STRENGTHEN_ELECTRIC` | Not audited | Unassigned | Not verified |
| 243 | Mystic Water | `ITEM_MYSTIC_WATER` | `HOLD_EFFECT_STRENGTHEN_WATER` | Not audited | Unassigned | Not verified |
| 244 | Sharp Beak | `ITEM_SHARP_BEAK` | `HOLD_EFFECT_STRENGTHEN_FLYING` | Not audited | Unassigned | Not verified |
| 245 | Poison Barb | `ITEM_POISON_BARB` | `HOLD_EFFECT_STRENGTHEN_POISON` | Not audited | Unassigned | Not verified |
| 246 | Never-Melt Ice | `ITEM_NEVER_MELT_ICE` | `HOLD_EFFECT_STRENGTHEN_ICE` | Not audited | Unassigned | Not verified |
| 247 | Spell Tag | `ITEM_SPELL_TAG` | `HOLD_EFFECT_STRENGTHEN_GHOST` | Not audited | Unassigned | Not verified |
| 248 | Twisted Spoon | `ITEM_TWISTED_SPOON` | `HOLD_EFFECT_STRENGTHEN_PSYCHIC` | Not audited | Unassigned | Not verified |
| 249 | Charcoal | `ITEM_CHARCOAL` | `HOLD_EFFECT_STRENGTHEN_FIRE` | Not audited | Unassigned | Not verified |
| 250 | Dragon Fang | `ITEM_DRAGON_FANG` | `HOLD_EFFECT_STRENGTHEN_DRAGON` | Not audited | Unassigned | Not verified |
| 251 | Silk Scarf | `ITEM_SILK_SCARF` | `HOLD_EFFECT_STRENGTHEN_NORMAL` | Not audited | Unassigned | Not verified |
| 252 | Up-Grade | `ITEM_UP_GRADE` | `HOLD_EFFECT_EVOLVE_PORYGON` | Not audited | Sage Neal (ID 55), Sprout Tower, x1 | Not verified |
| 253 | Shell Bell | `ITEM_SHELL_BELL` | `HOLD_EFFECT_HP_RESTORE_ON_DMG` | Not audited | Unassigned | Not verified |
| 254 | Sea Incense | `ITEM_SEA_INCENSE` | `HOLD_EFFECT_STRENGTHEN_WATER` | Not audited | Unassigned | Not verified |
| 255 | Lax Incense | `ITEM_LAX_INCENSE` | `HOLD_EFFECT_ACC_REDUCE` | Not audited | Bug Catcher Benny (ID 67), Azalea Gym, x1 | Not verified |
| 256 | Lucky Punch | `ITEM_LUCKY_PUNCH` | `HOLD_EFFECT_CHANSEY_CRITRATE_UP` | Not audited | Unassigned | Not verified |
| 257 | Metal Powder | `ITEM_METAL_POWDER` | `HOLD_EFFECT_DITTO_DEF_UP` | Not audited | Unassigned | Not verified |
| 258 | Thick Club | `ITEM_THICK_CLUB` | `HOLD_EFFECT_CUBONE_ATK_UP` | Not audited | Unassigned | Not verified |
| 259 | Leek | `ITEM_LEEK` | `HOLD_EFFECT_FARFETCHD_CRITRATE_UP` | Not audited | Unassigned | Not verified |
| 265 | Wide Lens | `ITEM_WIDE_LENS` | `HOLD_EFFECT_ACCURACY_UP` | Not audited | Unassigned | Not verified |
| 266 | Muscle Band | `ITEM_MUSCLE_BAND` | `HOLD_EFFECT_POWER_UP_PHYS` | Not audited | Unassigned | Not verified |
| 267 | Wise Glasses | `ITEM_WISE_GLASSES` | `HOLD_EFFECT_POWER_UP_SPEC` | Not audited | Unassigned | Not verified |
| 268 | Expert Belt | `ITEM_EXPERT_BELT` | `HOLD_EFFECT_POWER_UP_SE` | Not audited | Unassigned | Not verified |
| 269 | Light Clay | `ITEM_LIGHT_CLAY` | `HOLD_EFFECT_EXTEND_SCREENS` | Not audited | Unassigned | Not verified |
| 270 | Life Orb | `ITEM_LIFE_ORB` | `HOLD_EFFECT_HP_DRAIN_ON_ATK` | Not audited | Unassigned | Not verified |
| 271 | Power Herb | `ITEM_POWER_HERB` | `HOLD_EFFECT_CHARGE_SKIP` | Not audited | Unassigned | Not verified |
| 272 | Toxic Orb | `ITEM_TOXIC_ORB` | `HOLD_EFFECT_PSN_USER` | Not audited | Unassigned | Not verified |
| 273 | Flame Orb | `ITEM_FLAME_ORB` | `HOLD_EFFECT_BRN_USER` | Not audited | Unassigned | Not verified |
| 274 | Quick Powder | `ITEM_QUICK_POWDER` | `HOLD_EFFECT_DITTO_SPEED_UP` | Not audited | Unassigned | Not verified |
| 275 | Focus Sash | `ITEM_FOCUS_SASH` | `HOLD_EFFECT_ENDURE` | Not audited | Bird Keeper Rod (ID 29), Violet Gym, x6 | Not verified |
| 276 | Zoom Lens | `ITEM_ZOOM_LENS` | `HOLD_EFFECT_ACCURACY_UP_SLOWER` | Not audited | Unassigned | Not verified |
| 277 | Metronome | `ITEM_METRONOME` | `HOLD_EFFECT_BOOST_REPEATED` | Not audited | Unassigned | Not verified |
| 278 | Iron Ball | `ITEM_IRON_BALL` | `HOLD_EFFECT_SPEED_DOWN_GROUNDED` | Not audited | Unassigned | Not verified |
| 279 | Lagging Tail | `ITEM_LAGGING_TAIL` | `HOLD_EFFECT_PRIORITY_DOWN` | Not audited | Unassigned | Not verified |
| 280 | Destiny Knot | `ITEM_DESTINY_KNOT` | `HOLD_EFFECT_RECIPROCATE_INFAT` | Not audited | Unassigned | Not verified |
| 281 | Black Sludge | `ITEM_BLACK_SLUDGE` | `HOLD_EFFECT_HP_RESTORE_PSN_TYPE` | Not audited | Unassigned | Not verified |
| 282 | Icy Rock | `ITEM_ICY_ROCK` | `HOLD_EFFECT_EXTEND_HAIL` | Not audited | Unassigned | Not verified |
| 283 | Smooth Rock | `ITEM_SMOOTH_ROCK` | `HOLD_EFFECT_EXTEND_SANDSTORM` | Not audited | Unassigned | Not verified |
| 284 | Heat Rock | `ITEM_HEAT_ROCK` | `HOLD_EFFECT_EXTEND_SUN` | Not audited | Unassigned | Not verified |
| 285 | Damp Rock | `ITEM_DAMP_ROCK` | `HOLD_EFFECT_EXTEND_RAIN` | Not audited | Unassigned | Not verified |
| 286 | Grip Claw | `ITEM_GRIP_CLAW` | `HOLD_EFFECT_EXTEND_TRAPPING` | Not audited | Unassigned | Not verified |
| 287 | Choice Scarf | `ITEM_CHOICE_SCARF` | `HOLD_EFFECT_CHOICE_SPEED` | Not audited | Unassigned | Not verified |
| 288 | Sticky Barb | `ITEM_STICKY_BARB` | `HOLD_EFFECT_DMG_USER_CONTACT_XFR` | Not audited | Unassigned | Not verified |
| 289 | Power Bracer | `ITEM_POWER_BRACER` | `HOLD_EFFECT_LVLUP_ATK_EV_UP` | Not audited | Unassigned | Not verified |
| 290 | Power Belt | `ITEM_POWER_BELT` | `HOLD_EFFECT_LVLUP_DEF_EV_UP` | Not audited | Unassigned | Not verified |
| 291 | Power Lens | `ITEM_POWER_LENS` | `HOLD_EFFECT_LVLUP_SPATK_EV_UP` | Not audited | Unassigned | Not verified |
| 292 | Power Band | `ITEM_POWER_BAND` | `HOLD_EFFECT_LVLUP_SPDEF_EV_UP` | Not audited | Unassigned | Not verified |
| 293 | Power Anklet | `ITEM_POWER_ANKLET` | `HOLD_EFFECT_LVLUP_SPEED_EV_UP` | Not audited | Unassigned | Not verified |
| 294 | Power Weight | `ITEM_POWER_WEIGHT` | `HOLD_EFFECT_LVLUP_HP_EV_UP` | Not audited | Unassigned | Not verified |
| 295 | Shed Shell | `ITEM_SHED_SHELL` | `HOLD_EFFECT_SWITCH` | Not audited | Unassigned | Not verified |
| 296 | Big Root | `ITEM_BIG_ROOT` | `HOLD_EFFECT_LEECH_BOOST` | Not audited | Unassigned | Not verified |
| 297 | Choice Specs | `ITEM_CHOICE_SPECS` | `HOLD_EFFECT_CHOICE_SPATK` | Not audited | Unassigned | Not verified |
| 298 | Flame Plate | `ITEM_FLAME_PLATE` | `HOLD_EFFECT_ARCEUS_FIRE` | Not audited | Unassigned | Not verified |
| 299 | Splash Plate | `ITEM_SPLASH_PLATE` | `HOLD_EFFECT_ARCEUS_WATER` | Not audited | Unassigned | Not verified |
| 300 | Zap Plate | `ITEM_ZAP_PLATE` | `HOLD_EFFECT_ARCEUS_ELECTRIC` | Not audited | Unassigned | Not verified |
| 301 | Meadow Plate | `ITEM_MEADOW_PLATE` | `HOLD_EFFECT_ARCEUS_GRASS` | Not audited | Unassigned | Not verified |
| 302 | Icicle Plate | `ITEM_ICICLE_PLATE` | `HOLD_EFFECT_ARCEUS_ICE` | Not audited | Unassigned | Not verified |
| 303 | Fist Plate | `ITEM_FIST_PLATE` | `HOLD_EFFECT_ARCEUS_FIGHTING` | Not audited | Unassigned | Not verified |
| 304 | Toxic Plate | `ITEM_TOXIC_PLATE` | `HOLD_EFFECT_ARCEUS_POISON` | Not audited | Unassigned | Not verified |
| 305 | Earth Plate | `ITEM_EARTH_PLATE` | `HOLD_EFFECT_ARCEUS_GROUND` | Not audited | Unassigned | Not verified |
| 306 | Sky Plate | `ITEM_SKY_PLATE` | `HOLD_EFFECT_ARCEUS_FLYING` | Not audited | Unassigned | Not verified |
| 307 | Mind Plate | `ITEM_MIND_PLATE` | `HOLD_EFFECT_ARCEUS_PSYCHIC` | Not audited | Unassigned | Not verified |
| 308 | Insect Plate | `ITEM_INSECT_PLATE` | `HOLD_EFFECT_ARCEUS_BUG` | Not audited | Unassigned | Not verified |
| 309 | Stone Plate | `ITEM_STONE_PLATE` | `HOLD_EFFECT_ARCEUS_ROCK` | Not audited | Unassigned | Not verified |
| 310 | Spooky Plate | `ITEM_SPOOKY_PLATE` | `HOLD_EFFECT_ARCEUS_GHOST` | Not audited | Unassigned | Not verified |
| 311 | Draco Plate | `ITEM_DRACO_PLATE` | `HOLD_EFFECT_ARCEUS_DRAGON` | Not audited | Unassigned | Not verified |
| 312 | Dread Plate | `ITEM_DREAD_PLATE` | `HOLD_EFFECT_ARCEUS_DARK` | Not audited | Unassigned | Not verified |
| 313 | Iron Plate | `ITEM_IRON_PLATE` | `HOLD_EFFECT_ARCEUS_STEEL` | Not audited | Unassigned | Not verified |
| 314 | Odd Incense | `ITEM_ODD_INCENSE` | `HOLD_EFFECT_STRENGTHEN_PSYCHIC` | Not audited | Unassigned | Not verified |
| 315 | Rock Incense | `ITEM_ROCK_INCENSE` | `HOLD_EFFECT_STRENGTHEN_ROCK` | Not audited | Unassigned | Not verified |
| 316 | Full Incense | `ITEM_FULL_INCENSE` | `HOLD_EFFECT_PRIORITY_DOWN` | Not audited | Sage Edmond (ID 52), Sprout Tower, x1 | Not verified |
| 317 | Wave Incense | `ITEM_WAVE_INCENSE` | `HOLD_EFFECT_STRENGTHEN_WATER` | Not audited | Unassigned | Not verified |
| 318 | Rose Incense | `ITEM_ROSE_INCENSE` | `HOLD_EFFECT_STRENGTHEN_GRASS` | Not audited | Unassigned | Not verified |
| 319 | Luck Incense | `ITEM_LUCK_INCENSE` | `HOLD_EFFECT_MONEY_UP` | Not audited | Unassigned | Not verified |
| 320 | Pure Incense | `ITEM_PURE_INCENSE` | `HOLD_EFFECT_ENCOUNTERS_DOWN` | Not audited | Unassigned | Not verified |
| 321 | Protector | `ITEM_PROTECTOR` | `HOLD_EFFECT_EVOLVE_RHYDON` | Not audited | Unassigned | Not verified |
| 322 | Electirizer | `ITEM_ELECTIRIZER` | `HOLD_EFFECT_EVOLVE_ELECTABUZZ` | Not audited | Unassigned | Not verified |
| 323 | Magmarizer | `ITEM_MAGMARIZER` | `HOLD_EFFECT_EVOLVE_MAGMAR` | Not audited | Unassigned | Not verified |
| 324 | Dubious Disc | `ITEM_DUBIOUS_DISC` | `HOLD_EFFECT_EVOLVE_PORYGON2` | Not audited | Sage Troy (ID 54), Sprout Tower, x1 | Not verified |
| 325 | Reaper Cloth | `ITEM_REAPER_CLOTH` | `HOLD_EFFECT_EVOLVE_DUSCLOPS` | Not audited | Unassigned | Not verified |
| 326 | Razor Claw | `ITEM_RAZOR_CLAW` | `HOLD_EFFECT_CRITRATE_UP` | Not audited | Unassigned | Not verified |
| 327 | Razor Fang | `ITEM_RAZOR_FANG` | `HOLD_EFFECT_SOMETIMES_FLINCH` | Not audited | Bird Keeper Abe (ID 50), Violet Gym, x1 | Not verified |
| 537 | Prism Scale | `ITEM_PRISM_SCALE` | `HOLD_EFFECT_EVOLVE_FEEBAS` | Not audited | Unassigned | Not verified |
| 538 | Eviolite | `ITEM_EVIOLITE` | `HOLD_EFFECT_EVIOLITE` | Professor Elm after showing the tracked hatched Togepi, x1 | Unassigned | Not verified |
| 539 | Float Stone | `ITEM_FLOAT_STONE` | `HOLD_EFFECT_HALVE_WEIGHT` | Not audited | Unassigned | Not verified |
| 540 | Rocky Helmet | `ITEM_ROCKY_HELMET` | `HOLD_EFFECT_DAMAGE_ON_CONTACT` | Not audited | Unassigned | Not verified |
| 541 | Air Balloon | `ITEM_AIR_BALLOON` | `HOLD_EFFECT_UNGROUND_DESTROYED_ON_HIT` | Not audited | Unassigned | Not verified |
| 542 | Red Card | `ITEM_RED_CARD` | `HOLD_EFFECT_FORCE_SWITCH_ON_DAMAGE` | Not audited | Unassigned | Not verified |
| 543 | Ring Target | `ITEM_RING_TARGET` | `HOLD_EFFECT_LOSE_TYPE_IMMUNITIES` | Not audited | Unassigned | Not verified |
| 544 | Binding Band | `ITEM_BINDING_BAND` | `HOLD_EFFECT_TRAPPING_DAMAGE_UP` | Not audited | Unassigned | Not verified |
| 545 | Absorb Bulb | `ITEM_ABSORB_BULB` | `HOLD_EFFECT_BOOST_SPECIAL_ATTACK_ON_WATER_HIT` | Not audited | Unassigned | Not verified |
| 546 | Cell Battery | `ITEM_CELL_BATTERY` | `HOLD_EFFECT_BOOST_ATK_ON_ELECTRIC_HIT` | Not audited | Unassigned | Not verified |
| 547 | Eject Button | `ITEM_EJECT_BUTTON` | `HOLD_EFFECT_SWITCH_OUT_WHEN_HIT` | Not audited | Unassigned | Not verified |
| 548 | Fire Gem | `ITEM_FIRE_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 549 | Water Gem | `ITEM_WATER_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 550 | Electric Gem | `ITEM_ELECTRIC_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 551 | Grass Gem | `ITEM_GRASS_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 552 | Ice Gem | `ITEM_ICE_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 553 | Fighting Gem | `ITEM_FIGHTING_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 554 | Poison Gem | `ITEM_POISON_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 555 | Ground Gem | `ITEM_GROUND_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 556 | Flying Gem | `ITEM_FLYING_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 557 | Psychic Gem | `ITEM_PSYCHIC_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 558 | Bug Gem | `ITEM_BUG_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 559 | Rock Gem | `ITEM_ROCK_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 560 | Ghost Gem | `ITEM_GHOST_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 561 | Dragon Gem | `ITEM_DRAGON_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 562 | Dark Gem | `ITEM_DARK_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 563 | Steel Gem | `ITEM_STEEL_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 564 | Normal Gem | `ITEM_NORMAL_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 639 | Weakness Policy | `ITEM_WEAKNESS_POLICY` | `HOLD_EFFECT_BOOST_ATK_AND_SPATK_ON_SE` | Not audited | Unassigned | Not verified |
| 640 | Assault Vest | `ITEM_ASSAULT_VEST` | `HOLD_EFFECT_SPDEF_BOOST_NO_STATUS_MOVES` | Not audited | Unassigned | Not verified |
| 644 | Pixie Plate | `ITEM_PIXIE_PLATE` | `HOLD_EFFECT_ARCEUS_FAIRY` | Not audited | Unassigned | Not verified |
| 646 | Whipped Dream | `ITEM_WHIPPED_DREAM` | `HOLD_EFFECT_EVOLVE_SWIRLIX` | Not audited | Unassigned | Not verified |
| 647 | Sachet | `ITEM_SACHET` | `HOLD_EFFECT_EVOLVE_SPRITZEE` | Not audited | Unassigned | Not verified |
| 648 | Luminous Moss | `ITEM_LUMINOUS_MOSS` | `HOLD_EFFECT_BOOST_SPECIAL_DEFENSE_ON_WATER_HIT` | Not audited | Unassigned | Not verified |
| 649 | Snowball | `ITEM_SNOWBALL` | `HOLD_EFFECT_BOOST_ATK_ON_ICE_HIT` | Not audited | Unassigned | Not verified |
| 650 | Safety Goggles | `ITEM_SAFETY_GOGGLES` | `HOLD_EFFECT_SPORE_POWDER_IMMUNITY` | Not audited | Unassigned | Not verified |
| 715 | Fairy Gem | `ITEM_FAIRY_GEM` | `HOLD_EFFECT_POWERING_UP_MOVE_ONCE` | Not audited | Unassigned | Not verified |
| 846 | Adrenaline Orb | `ITEM_ADRENALINE_ORB` | `HOLD_EFFECT_INTIMIDATE_BOOST_SPEED` | Not audited | Unassigned | Not verified |
| 879 | Terrain Extender | `ITEM_TERRAIN_EXTENDER` | `HOLD_EFFECT_EXTEND_TERRAIN` | Not audited | Unassigned | Not verified |
| 880 | Protective Pads | `ITEM_PROTECTIVE_PADS` | `HOLD_EFFECT_PREVENT_CONTACT_EFFECTS` | Not audited | Unassigned | Not verified |
| 881 | Electric Seed | `ITEM_ELECTRIC_SEED` | `HOLD_EFFECT_BOOST_DEF_ON_ELECRIC_TERRAIN` | Not audited | Unassigned | Not verified |
| 882 | Psychic Seed | `ITEM_PSYCHIC_SEED` | `HOLD_EFFECT_BOOST_SPDEF_ON_PSYCHIC_TERRAIN` | Not audited | Unassigned | Not verified |
| 883 | Misty Seed | `ITEM_MISTY_SEED` | `HOLD_EFFECT_BOOST_SPDEF_ON_MISTY_TERRAIN` | Not audited | Unassigned | Not verified |
| 884 | Grassy Seed | `ITEM_GRASSY_SEED` | `HOLD_EFFECT_BOOST_DEF_ON_GRASSY_TERRAIN` | Not audited | Unassigned | Not verified |
| 904 | Fighting Memory | `ITEM_FIGHTING_MEMORY` | `HOLD_EFFECT_FIGHTING_MEMORY` | Not audited | Unassigned | Not verified |
| 905 | Flying Memory | `ITEM_FLYING_MEMORY` | `HOLD_EFFECT_FLYING_MEMORY` | Not audited | Unassigned | Not verified |
| 906 | Poison Memory | `ITEM_POISON_MEMORY` | `HOLD_EFFECT_POISON_MEMORY` | Not audited | Unassigned | Not verified |
| 907 | Ground Memory | `ITEM_GROUND_MEMORY` | `HOLD_EFFECT_GROUND_MEMORY` | Not audited | Unassigned | Not verified |
| 908 | Rock Memory | `ITEM_ROCK_MEMORY` | `HOLD_EFFECT_ROCK_MEMORY` | Not audited | Unassigned | Not verified |
| 909 | Bug Memory | `ITEM_BUG_MEMORY` | `HOLD_EFFECT_BUG_MEMORY` | Not audited | Unassigned | Not verified |
| 910 | Ghost Memory | `ITEM_GHOST_MEMORY` | `HOLD_EFFECT_GHOST_MEMORY` | Not audited | Unassigned | Not verified |
| 911 | Steel Memory | `ITEM_STEEL_MEMORY` | `HOLD_EFFECT_STEEL_MEMORY` | Not audited | Unassigned | Not verified |
| 912 | Fire Memory | `ITEM_FIRE_MEMORY` | `HOLD_EFFECT_FIRE_MEMORY` | Not audited | Unassigned | Not verified |
| 913 | Water Memory | `ITEM_WATER_MEMORY` | `HOLD_EFFECT_WATER_MEMORY` | Not audited | Unassigned | Not verified |
| 914 | Grass Memory | `ITEM_GRASS_MEMORY` | `HOLD_EFFECT_GRASS_MEMORY` | Not audited | Unassigned | Not verified |
| 915 | Electric Memory | `ITEM_ELECTRIC_MEMORY` | `HOLD_EFFECT_ELECTRIC_MEMORY` | Not audited | Unassigned | Not verified |
| 916 | Psychic Memory | `ITEM_PSYCHIC_MEMORY` | `HOLD_EFFECT_PSYCHIC_MEMORY` | Not audited | Unassigned | Not verified |
| 917 | Ice Memory | `ITEM_ICE_MEMORY` | `HOLD_EFFECT_ICE_MEMORY` | Not audited | Unassigned | Not verified |
| 918 | Dragon Memory | `ITEM_DRAGON_MEMORY` | `HOLD_EFFECT_DRAGON_MEMORY` | Not audited | Unassigned | Not verified |
| 919 | Dark Memory | `ITEM_DARK_MEMORY` | `HOLD_EFFECT_DARK_MEMORY` | Not audited | Unassigned | Not verified |
| 920 | Fairy Memory | `ITEM_FAIRY_MEMORY` | `HOLD_EFFECT_FAIRY_MEMORY` | Not audited | Unassigned | Not verified |
| 1103 | Rusted Sword | `ITEM_RUSTED_SWORD` | `HOLD_EFFECT_TRANSFORM_ZACIAN` | Not audited | Unassigned | Not verified |
| 1104 | Rusted Shield | `ITEM_RUSTED_SHIELD` | `HOLD_EFFECT_TRANSFORM_ZAMAZENTA` | Not audited | Unassigned | Not verified |
| 1118 | Throat Spray | `ITEM_THROAT_SPRAY` | `HOLD_EFFECT_BOOST_SPATK_ON_SOUND_MOVE` | Not audited | Unassigned | Not verified |
| 1119 | Eject Pack | `ITEM_EJECT_PACK` | `HOLD_EFFECT_SWITCH_OUT_ON_STAT_DROP` | Not audited | Unassigned | Not verified |
| 1120 | Heavy-Duty Boots | `ITEM_HEAVY_DUTY_BOOTS` | `HOLD_EFFECT_IGNORE_ENTRY_HAZARDS` | Not audited | Unassigned | Not verified |
| 1121 | Blunder Policy | `ITEM_BLUNDER_POLICY` | `HOLD_EFFECT_BOOST_SPEED_ON_MISS` | Not audited | Unassigned | Not verified |
| 1122 | Room Service | `ITEM_ROOM_SERVICE` | `HOLD_EFFECT_DROP_SPEED_IN_TRICK_ROOM` | Not audited | Unassigned | Not verified |
| 1123 | Utility Umbrella | `ITEM_UTILITY_UMBRELLA` | `HOLD_EFFECT_UNAFFECTED_BY_RAIN_OR_SUN` | Not audited | Unassigned | Not verified |
| 1777 | Adamant Crystal | `ITEM_ADAMANT_CRYSTAL` | `HOLD_EFFECT_DIALGA_BOOST_AND_TRANSFORM` | Not audited | Unassigned | Not verified |
| 1778 | Lustrous Globe | `ITEM_LUSTROUS_GLOBE` | `HOLD_EFFECT_PALKIA_BOOST_AND_TRANSFORM` | Not audited | Unassigned | Not verified |
| 1779 | Griseous Core | `ITEM_GRISEOUS_CORE` | `HOLD_EFFECT_GIRATINA_BOOST_AND_TRANSFORM` | Not audited | Unassigned | Not verified |
| 1780 | Blank Plate | `ITEM_BLANK_PLATE` | `HOLD_EFFECT_ARCEUS_NORMAL` | Not audited | Unassigned | Not verified |
| 1880 | Booster Energy | `ITEM_BOOSTER_ENERGY` | `HOLD_EFFECT_ACTIVATE_PARADOX_ABILITIES` | Not audited | Unassigned | Not verified |
| 1881 | Ability Shield | `ITEM_ABILITY_SHIELD` | `HOLD_EFFECT_PREVENT_ABILITY_CHANGES` | Not audited | Unassigned | Not verified |
| 1882 | Clear Amulet | `ITEM_CLEAR_AMULET` | `HOLD_EFFECT_PREVENT_STAT_DROPS` | Not audited | Unassigned | Not verified |
| 1883 | Mirror Herb | `ITEM_MIRROR_HERB` | `HOLD_EFFECT_COPY_STAT_INCREASE` | Not audited | Unassigned | Not verified |
| 1884 | Punching Glove | `ITEM_PUNCHING_GLOVE` | `HOLD_EFFECT_INCREASE_PUNCHING_MOVE_DMG` | Not audited | Unassigned | Not verified |
| 1885 | Covert Cloak | `ITEM_COVERT_CLOAK` | `HOLD_EFFECT_PREVENT_SECONDARY_EFFECTS` | Not audited | Unassigned | Not verified |
| 1886 | Loaded Dice | `ITEM_LOADED_DICE` | `HOLD_EFFECT_INCREASE_MULTI_STRIKE_MINIMUM` | Not audited | Unassigned | Not verified |
| 2401 | Fairy Feather | `ITEM_FAIRY_FEATHER` | `HOLD_EFFECT_STRENGTHEN_FAIRY` | Not audited | Unassigned | Not verified |
| 2406 | Cornerstone Mask | `ITEM_CORNERSTONE_MASK` | `HOLD_EFFECT_CORNERSTONE_MASK` | Not audited | Unassigned | Not verified |
| 2407 | Wellspring Mask | `ITEM_WELLSPRING_MASK` | `HOLD_EFFECT_WELLSPRING_MASK` | Not audited | Unassigned | Not verified |
| 2408 | Hearthflame Mask | `ITEM_HEARTHFLAME_MASK` | `HOLD_EFFECT_HEARTHFLAME_MASK` | Not audited | Unassigned | Not verified |

The authoritative candidate list is the set of non-Berry records whose
`holdEffect` is neither `0` nor `SOUBI_NONE` in
`data/itemdata/itemdata.c`. Recalculate the candidate count whenever item
behavior changes; do not infer candidacy from item names alone.
