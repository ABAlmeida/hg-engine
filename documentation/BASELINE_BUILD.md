# Baseline Build Record

## 2026-07-23 inspection

- Project: Pokémon Heartless Gold / hg-engine
- Host: Windows x64 (`Microsoft Windows NT 10.0.26200.0`)
- Shell inspected: Windows PowerShell 5.1
- Branch: `main`
- Revision: `b38bce6a76196111219275f248e57f7a9ece4448`
- Tracking state before documentation edits: clean,
  `main...origin/main`
- Origin: `https://github.com/ABAlmeida/hg-engine.git`
- Canonical hg-engine upstream named by repository documentation:
  `https://github.com/BluRosie/hg-engine.git`

## Inputs and dependencies

- Legal English US base ROM (`rom.nds`): present locally and identified only by
  game code `IPKE`, revision 0; the ROM is ignored by Git and is not recorded,
  hashed, copied into documentation, or redistributed
- Required nitrogfx submodule: initialized at
  `a8fd94a3e582ded71eda5b9f5f0c69258544bb22`
- MSYS2 UCRT64: GNU Make 4.4.1, ARM GCC 16.1.0, Python 3.12.13,
  CMake 4.4.0, and libpng 1.6.58
- Emulator: melonDS 1.1, software renderer, direct boot, external BIOS disabled
- Windows Git reports this checkout as having different filesystem ownership;
  inspection used a command-local `safe.directory` override without changing
  global Git configuration

## Baseline result

Status: **PASS**.

The build began from `make clean` and completed with exit code 0 using:

```sh
make -j$(nproc)
```

The command runner timed out during the long first build, so the same
incremental command was resumed until Make printed `Done. See output test.nds.`
The resulting local, ignored output is `test.nds` at repository root.

melonDS 1.1 booted the original ROM as a control. The first generated ROM
initially stopped on white screens with an ARM9 data abort at `0x0200B722`.
Debugging isolated this to message archive member 24, message 118: the Windows
build of `msgenc` collapsed intentional blank message records after CRLF
translation, so the game read past a 118-entry allocation table. The
cross-platform line parser now preserves empty records; the rebuilt member has
127 entries and message 118 has a valid length of 7.

After rebuilding the message archive and repacking, melonDS reached the normal
HeartGold intro at 60 FPS during a 30-second boot test. Its log contains
`Game is now booting` and no ARM9 data abort. melonDS still warns that the
expanded ROM's non-power-of-two file length is rounded to 256 MiB; this warning
did not prevent the verified boot.

No gameplay features were implemented during baseline setup.

## Reproduction

From an MSYS2 UCRT64 shell in repository root:

```sh
git submodule update --init --recursive
make clean
make -j$(nproc)
```

Expected output:

```text
test.nds
```

Then boot the ignored `test.nds` locally in melonDS 1.1 or newer. A fuller
release smoke test should additionally cover new-game start, overworld entry,
save, and reload.

Do not record ROM hashes, ROM contents, screenshots containing copyrighted
assets, or copies of either the base or generated ROM.
