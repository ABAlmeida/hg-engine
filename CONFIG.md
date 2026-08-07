# Configuration Options

### ``armips\include\config.s`` and ``include\config.h``
 ``DISABLE_BATTLE_EXPERIENCE`` (``include/config.h`` only) skips the complete battle experience sequence for player Pokemon, including its messages, bar animations, and level-ups. Unless ``DISABLE_BATTLE_EV_GAIN`` is also enabled, every non-empty party Pokemon with HP remaining still receives effort values. This option cannot be enabled together with ``IMPLEMENT_CAPTURE_EXPERIENCE``.

 ``DISABLE_BATTLE_EV_GAIN`` (``include/config.h`` only) prevents defeated Pokemon from awarding effort values while leaving vitamin use and EV reduction unchanged. It currently requires ``DISABLE_BATTLE_EXPERIENCE`` because the normal experience task owns vanilla battle EV distribution.

 ``DISALLOW_DEXIT_GEN`` (``armips/include/config.s`` only) controls whether to disallow selection of dexited moves in later generations, or disallow selection of unimplemented moves.

 ``FAIRY_TYPE_IMPLEMENTED`` should be set to 0 in both of these files in order to disable the fairy type implementation.  By default, it is 1 to implement the fairy type.

 ``ALLOW_SAVE_CHANGES`` is by default defined in these files to allow for save expansion for expanded dex Pokémon, bigger item pockets, and Kyurem's forme change method that stores Reshiram/Zekrom in the save.  If you want to maintain PKHeX compatibility, all you have to do is comment out the line ``ALLOW_SAVE_CHANGES`` in both of these files.  Commenting this line out disables Kyurem's forme change and the registering of new Pokémon in the dex (although the new mons are still implemented).

 ``CRY_PSEUDOBANK_START`` is where the pseudobanks start in the SDAT for new cries.  To save on sound heap RAM space, new banks are just detected and forced to load cries instead of having the massive structure that comes with it.  By default 778, which is the first empty index of SBNK's in the vanilla SDAT.

 ``BATTLE_MODE_FORCE_SET`` (``armips/include/config.s`` only) is the toggle to force set mode in the ROM.  Set to 1 to enable, 0 will use the default behavior.

 ``ALWAYS_HAVE_NATIONAL_DEX`` (``armips/include/config.s`` only) is the toggle that makes the player always have the national dex.  This will also affect things like the Bug Catching competition levels.

 ``ALWAYS_UNCAPPED_FRAME_RATE`` (``armips/include/config.s`` only) is the toggle that enables the traditional 60 fps hack that has a number of bugs.  Suggested for testing use only.

 ``BATTLES_UNCAPPED_FRAME_RATE`` (``armips/include/config.s`` only) is the toggle that enables a 60 fps hack that only activates in battles.  This will take precedence over the above if both are enabled for whatever reason.

 ``FAST_TEXT_PRINTING`` (``armips/include/config.s`` only) is a toggle that will force text to print at a really fast speed regardless of the setting.  Borderline instant.

 ``REUSABLE_TMS`` (``armips/include/config.s`` only) will make code edits to make TM's reusable.

 ``DELETABLE_HMS`` (``armips/include/config.s`` only) makes code edits to make HM's forgettable.  Strongly recommended at this time to pair with ``REUSABLE_TMS`` in order to preserve the HM's in the bag.

 ``NO_PARTNER_DOUBLE_BATTLES`` (``armips/include/config.s`` only) removes the requirement to put a partner NPC in the overworld for trainers with double battles. Trainer definitions and their text now live in ``data/Trainers.c``; double-battle defeat text should use the corresponding ``TRMSG_LOSE`` entry there.

 ``APPLY_ANTIPIRACY`` (``armips/include/config.s`` only) is a toggle that will apply the typical anti-piracy code changes to your ROM.  This will allow it to work on hardware (R4, TWLmenu) better, preventing various crashes and softlocks.

 ``EXPERIENCE_FORMULA_GEN`` (``include/config.h`` only) is a number that determines whether or not the experience formula used will take into account the difference in levels in experience distribution.  Setting this to 5, 7, or 8 will scale the experience with the level difference.  Setting this to 1, 2, 3, 4, and 6 will use a flat experience rate.

 ``HIDDEN_ABILITIES`` (``include/config.h`` only) is a toggle for the compilation of the hidden ability system.  Commenting the line out will disable hidden abilities.  ``HIDDEN_ABILITIES_FLAG`` determines which script flag that, when set, will give Pokémon their hidden abilities (unset immediately after giving a Pokémon its hidden ability).  ``HIDDEN_ABILITIES_STARTERS_FLAG`` is specific to the starters, and can be used as a story flag after the initial starter is given.

 ``MEGA_EVOLUTIONS`` (``include/config.h`` only) is a toggle for the compilation of the mega evolution system.  Commenting the line out will disable mega evolutions.

 ``PRIMAL_REVERSION`` (``include/config.h`` only) is a toggle for the compilation of the primal reversion system.  Commenting the line out will disable Groudon and Kyogre from primally reverting under any circumstances.

 ``IMPLEMENT_BDHCAM_ROUTINE`` (``include/config.h`` only) is a toggle for the assembly of [Mikelan's BDHCAM system](https://pokehacking.com/r/20110901) into overlay 131.  This is because hg-engine does not use the synthetic overlay system, so it needs to dynamically link it itself.

 ``IMPLEMENT_TRANSPARENT_TEXTBOXES`` (``include/config.h`` only) is a toggle for the compilation and inclusion of transparent textboxes in your hack.

 ``IMPLEMENT_WILD_DOUBLE_BATTLES`` (``include/config.h`` only) is a toggle for the compilation of wild double battles to occur 10% of the time in the grass.

 ``IMPLEMENT_CAPTURE_EXPERIENCE`` (``include/config.h`` only) is a toggle that will give Pokémon that participated in battle experience when a Pokémon is caught.

 ``IMPLEMENT_CRITICAL_CAPTURE`` (``include/config.h`` only) is a toggle that will enable critical captures as they appear in Gen 5 and above--the more Pokémon you have caught, the more likely that a critical capture will occur.

 ``IMPLEMENT_NEW_EV_IV_VIEWER`` (``include/config.h`` only) is a toggle that will enable an EV/IV viewer on the summary screen using the L, R, and Select buttons to swap between the EV's, IV's, and normal stats respectively.  This also adds up and down arrows to the stat names to make it clear which is boosted and which is nerfed.

 ``IMPLEMENT_LEVEL_CAP`` (``include/config.h`` only) enables the Heartless Gold hard level cap. The current cap is stored in the expanded save data, starts at level 7, and is raised by configured story-trainer victories. Lower cap rewards are ignored. Eligible Pokémon below the cap gain a ``LEVEL TO CAP`` command in their normal party-menu actions. It advances one level at a time, runs every move-learning and evolution prompt, and stops at the saved cap without consuming an item. If a Pokémon is at or above the cap, experience gain is disabled and Rare Candies stop working. ``UNCAP_CANDIES_FROM_LEVEL_CAP`` further decouples Rare Candies from this requirement, allowing Rare Candies to level Pokémon beyond the level cap. Contrarily, ``ALLOW_LEVEL_CAP_EVOLVE`` allows Rare Candies to trigger an evolution if the Pokémon would otherwise have evolved upon leveling up. See ``documentation/LEVEL_CAPS.md`` for the progression table and command behavior.

 ``IMPLEMENT_INSTANT_EGG_HATCH`` (``include/config.h`` only) adds a ``HATCH`` command to an Egg's normal Party-menu actions. It closes the Party menu, makes the selected Egg ready, and starts HGSS's standard hatch sequence, including its animation, Pokédex update, statistics, and nickname prompt.

 ``UPDATE_OVERWORLD_POISON`` (``include/config.h`` only) is a toggle that disables overworld poison when enabled.  Overworld poison is disabled by default.

 ``DISABLE_END_OF_TURN_WEATHER_MESSAGE`` (``include/config.h`` only) will remove end-of-turn weather printing for everything but Hail and Sandstorm, which still play the animation before harming the Pokémon.  There is a small overlay over the Fight button to show which weather is active.  The config defaults to keeping the weather printing at the end of the turn.

 ``EXPAND_PC_BOXES`` (``include/config.h`` only) will expand the PC boxes to 30 in the game from the original 18.  Boxes are expanded to 30 by default.

 ``FRIENDSHIP_EVOLUTION_THRESHOLD`` (``include/config.h`` only) defines the threshold for evolutions that involve friendship to start happening.  This is vanilla 220, but in modern generations is 160.

 ``RESTORE_ITEMS_AT_BATTLE_END`` (``include/config.h`` only) will restore most single-use items at the end of the battle if defined.  This does not include held berries per modern generation standards.

 ``PROTEAN_GENERATION`` (``include/config.h`` only) defines the generation that Protean's behavior is implemented from.  Later generations make Protean only activate once per appearance in battle.

 ``IMPLEMENT_BAIT_ENCOUNTERS`` (``include/config.h``) disables ordinary passive land and surfing encounters and enables the custom Poké Bait items. Poké Bait starts one ordinary encounter from the current terrain's normal table; Shiny Bait does the same and assigns a genuinely shiny personality value. Roamers, Safari Zone, Bug-Catching Contest, fishing, Rock Smash, Headbutt, and static or scripted encounters keep their normal entry points. This option replaces ``IMPLEMENT_REUSABLE_REPELS``; the two systems are not compatible. The build generates the matching Armips setting automatically, so commenting out the C definition restores passive encounters, the original Repel metadata and acquisition sources, and the original Rare Candy rewards. See ``documentation/BAIT_ENCOUNTERS.md`` for implementation and acquisition details.

 ``DISABLE_TRAINER_LINE_OF_SIGHT`` (``include/config.h``) prevents field trainers from automatically spotting, approaching, or interrupting the player. The shared first-time talk interaction shows the trainer's generated team and configured reward, then asks whether to battle. Trainer-defined single and double battles, defeated dialogue, phone interactions, and rematches retain their existing paths. Automatic and custom map-script battles are not given this offer. Separate trainers that previously joined one battle only because their sight lines overlapped are fought individually.

 ``IMPLEMENT_MACHINE_FIELD_ACTIONS`` (``include/config.h`` only) lets an owned
 HM provide Cut, Fly, Surf, Strength, Whirlpool, Rock Smash, Waterfall, or Rock
 Climb without teaching it to a compatible Pokémon. Owned TM70 provides Flash
 in the same way. Existing badge, terrain, map, follower, costume, Safari, and
 Pal Park restrictions remain in force. Direct obstacle interactions use the
 first non-Egg party Pokémon when no party member knows the move; context-valid
 Party-menu actions use the selected non-Egg Pokémon. HM and TM teaching
 behavior is unchanged. See ``documentation/HM_FIELD_ACTIONS_PLAN.md``.

 ``UPDATE_VITAMIN_EV_CAPS`` (``include/config.h`` only) enables the centralized stat-training item rules: vitamins can reach 252 EVs per stat, the unsuffixed vitamins give 50 EVs, the S/L/Max tiers give 10/100/up to the legal maximum, and IV Max sets all six IVs to 31.

 ``DISABLE_ITEMS_IN_TRAINER_BATTLE`` (``include/config.h`` only) disables items being used while in trainer battles.

 ``STATIC_HP_BAR`` (``include/config.h`` only) updates the HP bar to increase/decrease at a fixed rate like later generations.

### ``scripts\make.py`` and ``linker.ld``
 On line 13 of ``scripts/make.py``, change ``OFFSET_TO_START`` to be your location in overlay 129 with enough free space.  This is separate from the above one.  MAKE SURE THE OFFSETS DON'T OVERLAP.

 Change the numbers at the beginning of ``linker.ld`` to be the same thing.

 NOTE:  It is strongly suggested that code edits and repoints be done through hg-engine since its move to get rid of the synthetic overlay.
