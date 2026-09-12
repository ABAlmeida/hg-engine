# Build Targets

Run build commands from the repository root in MSYS2 UCRT64, with
`MINGW_PREFIX=/ucrt64` and `/ucrt64/bin` before `/usr/bin`.

Use the narrowest target that covers the change:

```sh
make -j$(nproc)
```

Uses the default full build for an explicitly requested clean baseline or when
the repository's normal entry point is specifically required.

```sh
make code -j$(nproc)
```

Compiles and links injected code without modifying `base/` or packaging a ROM.

```sh
make quick-rom -j$(nproc)
```

Runs the safe incremental, timestamp-driven ROM graph. This is the normal
user-facing ROM build.

```sh
make full-rom -j$(nproc)
```

Uses the full normal dependency graph when it is specifically required.
`quick-rom` and `full-rom` currently share the same safe timestamp-driven
dependency graph; the separate names communicate intent.

```sh
make rebuild_scripts
```

Use only when deliberately importing edits made directly to the installed
DSPRE script archive. Normal source-controlled script changes do not require
this target.

Do not recommend `make clean` for convenience. Prefer `make clean_code` when
only compiled C or assembly outputs need regeneration. Explain the cost and
reason before recommending broader cleanup.

For ROM targets, the expected ignored output is `test.nds`. Report the command
used or recommended, why it covers the changes, and which focused in-game
checks the user should perform. Never claim the build or checks passed unless
the result is available, and never run the game or emulator yourself.
