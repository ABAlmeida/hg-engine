# Pokémon Heartless Gold Repository Guide

This repository is the source of truth for Pokémon Heartless Gold. Make every
change reproducible from source; never rely on edits that exist only in a
generated ROM or an emulator save.

## Repository layout and upstream

- Run project commands from this directory (`hg-engine`), not its parent.
- The project currently tracks branch `main` at
  `b38bce6a76196111219275f248e57f7a9ece4448`.
- The configured `origin` is `https://github.com/ABAlmeida/hg-engine.git`.
  hg-engine's canonical upstream is `https://github.com/BluRosie/hg-engine.git`,
  but no local `upstream` remote is currently configured.
- `tools/source/nitrogfx` is a required Git submodule. Initialize it with:

  ```sh
  git submodule update --init --recursive
  ```

## Supported build environment

hg-engine targets the English US Pokémon HeartGold ROM (game code `IPKE`). On
Windows, use WSL2 or the MSYS2 UCRT64 shell; the Makefile is a Unix make
workflow and is not a native PowerShell build. Current MSYS2 installs the ARM
toolchain as `mingw-w64-ucrt-x86_64-arm-none-eabi-toolchain`; do not use the
obsolete `mingw-w64-x86_64-arm-none-eabi-gcc` package name. Verify that
`which cmake` returns `/ucrt64/bin/cmake`; MSYS `/usr/bin/cmake` misclassifies
the UCRT compiler when building armips.

Required host tools are documented in `README.md`. The practical dependency
set is:

- GNU Make and a native C/C++ build toolchain
- `arm-none-eabi` binutils/GCC
- Python 3 with `venv`/`pip`
- UCRT64 CMake (`/ucrt64/bin/cmake`), Git, autoconf, automake, UCRT64
  pkg-config, and UCRT64 libpng development files
- 7-Zip where required by the selected setup path
- clang-format 18 (preferred) or clang-format for C/header changes

The first build also needs network access to install Python requirements and
fetch build tools that are not cached locally. The repository provides a
Docker workflow as an alternative, but WSL2 is preferred for day-to-day
development on Windows.

## ROM handling

- Supply your own legally obtained English US Pokémon HeartGold ROM.
- Keep it locally at repository root as `rom.nds`; never commit, upload,
  redistribute, print, hash, or copy it into documentation or logs.
- Never commit generated `.nds`/`.srl` files, emulator saves, save states,
  extracted `base/` files, `build/` files, or other copyrighted/generated
  output.
- Do not commit `test.nds`. Distribute source patches or other lawful,
  source-derived artifacts instead of a modified full ROM.
- Do not commit test saves unless their provenance and redistribution rights
  are explicitly established.

## Build commands

Initialize the submodule once, place the legal base ROM as `rom.nds`, then run:

```sh
make -j$(nproc)
```

The generated ROM is `test.nds` at repository root.

For a clean baseline rebuild:

```sh
make clean
make -j$(nproc)
```

For a toolchain-from-scratch rebuild:

```sh
make clean
make clean_tools
git submodule update --init --recursive
make -j$(nproc)
```

Use `make clean_code` when only compiled C/assembly outputs need rebuilding.
Do not use `make restore` unless the required local `romClean.nds` workflow has
been deliberately set up; ROM backup copies remain local and ignored.

## Testing commands

Build the automated battle-test ROM and run the headless suite:

```sh
make clean
make AUTO_TEST=Y -j$(nproc)
SDL_VIDEODRIVER=dummy scripts/run_tests.sh -c -j $(nproc)
```

Run tests with video using:

```sh
scripts/run_tests.sh -v
```

After a normal build, smoke-test `test.nds` in an accurate Nintendo DS
emulator such as melonDS. At minimum verify that it reaches the title screen,
starts a new game, enters the overworld, and can save and reload. Record the
emulator name/version and result in `documentation/BASELINE_BUILD.md`.

Feature changes need focused regression tests in addition to the baseline
smoke test. Battle-engine changes should add scenarios under
`data/battle_tests/` and follow that directory's `README.md`.

## Repository conventions

- Keep logical changes small and independently buildable.
- Before editing, run `git status --short --branch`; preserve unrelated user
  changes.
- Follow `.clang-format` for `src/**/*.c` and `include/**/*.h`. Configure the
  supplied hook with `git config core.hooksPath .githooks`.
- Prefer readable source data, PNGs, palettes, JSON layouts/animations,
  scripts, and tables over opaque binary replacements.
- Never make the only copy of a permanent change in DSPRE or a generated ROM.
- When a binary NARC member is unavoidable, commit only the smallest changed
  member and document the original archive path, member index, purpose,
  editing tool, and complete reproduction steps.
- Keep generated files out of Git. If a new tool creates output, add a narrow
  ignore rule rather than ignoring source formats globally.

## Relevant hg-engine notes

- `README.md`, `CONFIG.md`, and `CONTRIBUTING.md` are the primary setup and
  contribution references.
- `include/config.h` and `armips/include/config.s` are separate configuration
  surfaces; keep equivalent settings consistent where both exist.
- Expanded save support must remain enabled through `ALLOW_SAVE_CHANGES`;
  expanded PC boxes are enabled with `EXPAND_PC_BOXES`. Ordinary HeartGold
  saves and PKHeX compatibility are not project requirements.
- Capture experience is disabled. Keep `IMPLEMENT_CAPTURE_EXPERIENCE`
  undefined while the project implements its no-battle-EXP progression model.
- hg-engine has a script-variable level-cap option, but Heartless Gold requires
  a central story/badge-aware cap function and additional non-battle leveling
  integration.
- The upstream documentation describes the expanded Pokédex through Generation
  6 as almost complete, but every Generation 5 species, asset, evolution,
  learnset, form, cry, save, and Pokédex path still requires an explicit audit.
- New species being compiled does not place them in encounters, trainers,
  gifts, or other content.
- The existing wild-double-battle option is documented as unstable/broken and
  should remain disabled.

## Current baseline

See `documentation/BASELINE_BUILD.md`. The 2026-07-23 clean normal build and
melonDS 1.1 boot check pass. Keep that build-and-boot baseline green before
starting or merging gameplay feature work.
