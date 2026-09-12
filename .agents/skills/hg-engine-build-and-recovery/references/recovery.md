# Build Recovery

Inspect existing logs, timestamps, dependency rules, and source before choosing
a recovery action. Do not rerun a build unless the user explicitly authorizes
one. Keep large logs under `.scratch/` and quote only relevant excerpts.

## Narrow recovery first

- Use MSYS2 UCRT64 rather than native PowerShell Make.
- Confirm `/ucrt64/bin/cmake` is selected; `/usr/bin/cmake` can misclassify the
  UCRT compiler while building armips.
- Prefer `make clean_code` for stale compiled C or assembly outputs.
- Use `make clean` only when the normal dependency graph cannot repair stale
  generated data, and explain that it can trigger expensive regeneration.
- Use `make clean_tools` only for a demonstrated tool-build problem.
- Do not use `make restore` unless the local `romClean.nds` workflow was
  deliberately configured.
- Never use destructive Git recovery such as `git reset --hard` to repair line
  endings or generated output.

When the user explicitly requests a clean baseline build and the clean state is
material to the request, use:

```sh
make clean
make -j$(nproc)
```

For a demonstrated toolchain-from-scratch recovery, use:

```sh
make clean
make clean_tools
git submodule update --init --recursive
make -j$(nproc)
```

These sequences are exceptional recovery paths, not routine build advice. Do
not execute them without explicit build authorization.

## Known failure modes

Normal ROM builds must run `refresh_base_code` before binary patching. An older
`AUTO_TEST=Y` build may leave debug patches in extracted executable files;
omitting a disabled Armips patch does not restore the original bytes. The
refresh step restores ARM9, its overlay table, and overlays from the legal
local `rom.nds` while preserving generated NitroFS data.

CRLF-sensitive message inputs can corrupt generated message archives by
collapsing intentional blank records. Diagnose source line endings and archive
inputs without printing or documenting ROM contents. See
`documentation/BASELINE_BUILD.md` for the preserved historical incident and
verified recovery behavior. Keep that historical baseline unchanged; later
feature builds and manual checks belong in `documentation/PROJECT_PLAN.md`.
