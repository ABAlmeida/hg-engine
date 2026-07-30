---
name: hgss-scripting-rules
description: Safely plan, edit, diagnose, and review Pokémon HeartGold and SoulSilver DSPRE field scripts using verified HGSS commands, parameters, variables, flags, messages, maps, trainers, and objects. Use for changes to DSPRE scripts, Armips script sources or macros, field events, common scripts, script-triggered dialogue, and any investigation involving HGSS script commands or identifiers.
---

# HGSS Scripting Rules

Treat HGSS scripting as a version-specific, evidence-driven interface. Prefer
the smallest modification to an existing working flow and preserve reusable
knowledge under `examples/`.

## Required workflow

1. Read the relevant files under `examples/` before planning or editing. Search
   them with `rg` when the applicable example is not obvious.
2. Locate the target source-controlled script and trace its callers, branches,
   messages, variables, flags, maps, trainers, and objects.
3. Before using any command, find a working HGSS use in this repository. If
   none exists, verify it against the DSPRE HGSS script-command database.
4. Copy parameter count, ordering, width, and meaning from the verified HGSS
   example. Never infer them from the command name.
5. Search the project before reading or writing any variable or flag. Confirm
   its existing purpose and lifetime.
6. Inspect the target map's event data before referring to an NPC or object ID.
7. Verify every message, trainer, map, event, script, item, and other content
   ID from source or extracted project metadata. Never invent an ID.
8. Make the smallest viable change. Prefer extending a working script branch
   over replacing or restructuring the entire script.
9. Review control flow, lock/release balance, message close/wait behavior,
   fades, task ownership, termination, and all paths through the edited block.
10. Add or update a focused how-to under `examples/` in the same change. Do not
    consider a script edit complete without preserving the verified pattern.

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
HeartGold/SoulSilver. Record the database entry and HGSS applicability in the
corresponding example. If the database cannot be accessed or does not establish
the parameter contract, do not use the command.

## Examples knowledge base

Store one focused Markdown how-to per reusable scripting pattern under
`examples/`. Read [how-to-template.md](examples/how-to-template.md) before
creating a new entry.

Each how-to must record:

- the behavior and relevant HGSS script;
- every command used and its known-good HGSS source;
- parameter ordering and meanings;
- provenance for variables, flags, messages, trainers, maps, events, and
  objects;
- the minimal final pattern;
- control-flow and lifecycle risks;
- build instructions and focused manual checks.

Update an existing how-to instead of duplicating the same pattern. Explicitly
label anything not yet verified; examples are evidence records, not a place for
speculation.
