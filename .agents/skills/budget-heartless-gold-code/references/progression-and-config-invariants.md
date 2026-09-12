# Progression and Configuration Invariants

Read this reference only when changing configuration, save-backed progression,
capture experience, or level-cap behavior.

- `include/config.h` and `armips/include/config.s` are normally separate
  configuration surfaces. Keep equivalent settings consistent where both
  exist. Bait encounters are the deliberate exception: the build generates
  `build/armips_config.s` from `IMPLEMENT_BAIT_ENCOUNTERS` in
  `include/config.h`.
- Expanded save support must remain enabled through `ALLOW_SAVE_CHANGES`;
  expanded PC boxes use `EXPAND_PC_BOXES`. Ordinary HeartGold saves and PKHeX
  compatibility are not project requirements.
- Capture experience is disabled for the no-battle-EXP progression model. Keep
  `IMPLEMENT_CAPTURE_EXPERIENCE` undefined.
- The monotonic level cap is stored in expanded save data and raised from the
  central story-trainer victory table. Route all consumers through
  `GetLevelCap()`.
