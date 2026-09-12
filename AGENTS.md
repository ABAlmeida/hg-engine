# Pokémon Heartless Gold Repository Policy

This file is the canonical policy for work in this repository. More specific
skills add task workflows but do not override these rules.

## Universal boundaries

- Work from this `hg-engine` directory, not its parent. Before editing, inspect
  `git status --short --branch` and preserve unrelated user changes.
- Keep every permanent change reproducible from source. Do not rely on edits
  that exist only in a generated ROM, DSPRE workspace, emulator save, or save
  state.
- Never run Pokémon Heartless Gold automated tests, battle tests, test scripts,
  test ROMs, manual gameplay, or emulator smoke tests. Never launch an emulator
  or use `AUTO_TEST=Y`.
- Run Make, compile or link code, package a ROM, or create `test.nds` only when
  the user explicitly requests a build in the current conversation. A request
  to implement, fix, review, verify, or test does not authorize a build.
- Read-only inspection, static analysis, formatting checks, generator `--check`
  modes, and `git diff --check` are allowed. Do not describe them as a
  successful build or runtime test.
- Never commit, upload, redistribute, print, hash, quote, or document ROM
  contents, generated ROMs, saves, extracted `base/` files, or copyrighted
  screenshots. Keep diagnostics and disposable artifacts under ignored
  `.scratch/` paths.

## Normal workflow

- Begin with targeted `rg` searches, diff summaries, and bounded excerpts.
  Keep large logs out of conversation and inspect only relevant sections.
- Trace the existing call flow and prefer established engine mechanisms,
  centralized logic, named constants, source-controlled data, and deterministic
  behavior. Avoid fragile hooks, magic addresses, duplicated state, and unsafe
  lifetime assumptions.
- After changing files, review the complete resulting diff and interaction
  between changed files. Check correctness, edge cases, configuration guards,
  readability, maintainability, comments, and relevant CPU, memory, allocation,
  hot-path, binary-patch, structure-layout, and call-timing risks.
- Fix in-scope review findings and report remaining risks or decisions. Do not
  claim runtime correctness without a result supplied by the user.
- Follow `.clang-format` for `src/**/*.c` and `include/**/*.h`. Keep generated
  output out of Git and prefer readable source formats over opaque binary
  replacements. If an opaque member is unavoidable, track only the smallest
  required unit and record its archive, member index, editing tool, and
  reproduction method; never track a generated full archive or ROM output.

## Subagent delegation

- Delegate only independent, bounded work when parallel execution or a
  smaller model is likely to improve speed or quality. Do not delegate simple
  edits, tightly coupled debugging, architecture, final integration, or work
  that depends on unstated conversation context.
- When supported, spawn subagents without conversation history. Give each one
  a self-contained prompt with the repository root, exact scope, constraints,
  authoritative sources, expected output, and stopping condition.
- Use `hg_scan` for mechanical searches and comparisons, `hg_inspector` for
  bounded code or documentation analysis, and `hg_worker` for one isolated
  implementation with an exclusive set of files.
- Spawn no more agents than there are independent workstreams. Subagents must
  not delegate further. At most one write-capable subagent may run at a time,
  and the main agent must not edit that subagent's owned files concurrently.
- Subagents must never build, test, inspect ROM or generated-ROM content,
  launch an emulator, or commit. The main agent retains any explicitly
  authorized build, architectural decisions, and final review; repository-wide
  prohibitions still apply. Verify material subagent findings against the
  current working tree before relying on them.

## Conditional workflows

Before planning or editing, read every applicable skill below and no unrelated
skill. Their frontmatter descriptions define the full activation boundary.

- Explicit builds, build instructions, toolchain setup, or build failures:
  `.agents/skills/hg-engine-build-and-recovery/SKILL.md`
- Injected C/assembly, hooks, linker space, save layout, heap, stack, VRAM, or
  materially large archives/assets:
  `.agents/skills/budget-heartless-gold-code/SKILL.md`
- HGSS field/DSPRE scripts, field events, or field-script identifiers:
  `.agents/skills/hgss-scripting-rules/SKILL.md`
- Species data, ability limitations, move implementation flags, learnsets,
  encounters, or trainer parties:
  `.agents/skills/maintain-pokemon-availability/SKILL.md`
- First-victory trainer reward additions, removals, reassignment, quantity,
  naming, or verification:
  `.agents/skills/maintain-trainer-reward-trackers/SKILL.md`
- Applying `../Pokemon Trainers.txt` or `../Pokemon Encounters.txt` drafts:
  `.agents/skills/import-heartless-gold-content-drafts/SKILL.md`

For upstream integration or remote history, read
`documentation/guides/REPOSITORY_CONTEXT.md`. Human setup belongs in
`README.md`; historical build evidence belongs in
`documentation/BASELINE_BUILD.md`.
