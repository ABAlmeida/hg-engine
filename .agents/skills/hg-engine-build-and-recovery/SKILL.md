---
name: hg-engine-build-and-recovery
description: Select or diagnose hg-engine build workflows. Use only for an explicit build request, build instructions, toolchain setup, or a reported compile, link, packaging, or ROM-build failure; do not use merely because source files changed.
---

# hg-engine Build and Recovery

Follow the repository `AGENTS.md`. This skill does not authorize a build,
Pokémon test, emulator use, or ROM inspection.

Choose only the material needed for the request:

- For an explicitly authorized build or a build-command handoff, read
  [build-targets.md](references/build-targets.md).
- For installing or checking the supported environment, read
  [toolchain.md](references/toolchain.md).
- For a reported build failure, inspect the supplied logs and source first,
  then read [recovery.md](references/recovery.md). Diagnosis alone does not
  authorize rerunning the build.

Use the narrowest workflow that addresses the request. Never treat a completed
build as a runtime test, and never launch an emulator. If files were changed
without build authorization, give the user the appropriate command and focused
manual checks instead of executing them.
