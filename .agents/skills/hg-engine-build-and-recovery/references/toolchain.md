# Supported Build Environment

hg-engine targets a legally obtained English US Pokémon HeartGold ROM with
game code `IPKE`. The local input is `rom.nds` at repository root. Do not copy,
hash, print, upload, commit, or redistribute it or generated ROM content.

Use WSL2 or the MSYS2 UCRT64 shell on Windows. The Makefile is a Unix Make
workflow, not a native PowerShell build. The repository's primary installation
instructions and current packages are in `README.md`.

For MSYS2, confirm:

- the shell is UCRT64;
- `which cmake` reports `/ucrt64/bin/cmake`, not `/usr/bin/cmake`;
- GNU Make, a native C/C++ toolchain, Python 3 with `venv`/`pip`, Git,
  autoconf, automake, UCRT64 pkg-config and libpng, and `arm-none-eabi` GCC and
  binutils are available; and
- `tools/source/nitrogfx` is initialized with
  `git submodule update --init --recursive`.

Use clang-format 18 when available for C and header changes. Configure the
supplied repository hook with `git config core.hooksPath .githooks` when the
user requests development-environment setup.

Current MSYS2 uses `mingw-w64-ucrt-x86_64-arm-none-eabi-toolchain`; do not
recommend the obsolete `mingw-w64-x86_64-arm-none-eabi-gcc` package. The first
build may require network access for Python requirements and uncached build
tools. Docker is supported by the repository, while WSL2 or UCRT64 is more
convenient for ordinary development.
