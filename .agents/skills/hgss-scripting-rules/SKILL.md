---
name: hgss-scripting-rules
description: Plan, edit, diagnose, or review Pokémon HGSS DSPRE field scripts, field events, common field scripts, dialogue, and their commands or identifiers. Do not use for battle scripts, move-effect scripts, or generic Armips assembly.
---

# HGSS Scripting Rules

The repository `AGENTS.md` remains authoritative. This skill grants no build,
test, ROM-inspection, or emulator authorization.

Treat HGSS field scripting as a version-specific, evidence-driven interface.
Prefer the smallest modification to an existing working flow.

## Workflow

1. Locate the target source-controlled script and trace its callers, branches,
   messages, variables, flags, maps, trainers, and objects.
2. Before using any command, find a working HGSS use in this repository. If
   none exists, verify it against the DSPRE HGSS script-command database.
3. Copy parameter count, ordering, width, and meaning from the verified HGSS
   example. Never infer them from the command name.
4. Search the project before reading or writing any variable or flag. Confirm
   its existing purpose and lifetime.
5. Inspect the target map's event data before referring to an NPC or object ID.
6. Verify every message, trainer, map, event, script, item, and other content
   ID from source or extracted project metadata. Never invent an ID.
7. Make the smallest viable change. Prefer extending a working script branch
   over replacing or restructuring the entire script.
8. Review control flow, lock/release balance, message close/wait behavior,
   fades, task ownership, termination, and all paths through the edited block.

## Absolute rules

- Never invent a script command.
- Never assume a Generation III or pokeemerald command exists in HGSS.
- Never assume a Pokémon Platinum command has identical HGSS behavior or
  parameters.
- Never guess an unverified value. Report the missing evidence and stop that
  part of the change.
- Do not treat a successful assembly or ROM build as proof that script control
  flow works in game.

## Repository verification

Use repository evidence first:

```sh
rg -n "<command-or-id>" armips data documentation include src
rg -n "<script-label-or-standard-script-id>" armips
```

Verify external documentation only against the DSPRE command database for
HeartGold/SoulSilver. If the database cannot be accessed or does not establish
the parameter contract, do not use the command.

## Knowledge placement

- Current source is the concrete example for an implemented script pattern.
- Put feature behavior, decisions, and pending manual scenarios in the
  feature's canonical plan, not in this skill.
- Put build and archive-recovery procedures in the build workflow selected by
  `AGENTS.md`; do not duplicate build recipes here.
- Update this skill only for a verified, non-obvious rule that changes future
  scripting decisions across multiple features and is not already clear from
  source or existing documentation.
- Do not create a how-to or instruction artifact merely because a script was
  edited. Git history preserves implementation history.

When an unusual command or caller contract needs explanation, prefer a concise
comment beside its source-controlled use. Avoid copying feature-specific
identifiers, code excerpts, or verification checklists into general agent
instructions.
