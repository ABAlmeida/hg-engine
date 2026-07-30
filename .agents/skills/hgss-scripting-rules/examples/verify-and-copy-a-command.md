# Verify and copy an HGSS script command

## Goal

Add a command to an HGSS script without guessing its availability or parameter
contract.

## Target

- Script archive/source: the source-controlled file being edited
- Script label or entry: the smallest existing branch that needs the behavior
- Trigger or caller: verify before editing

## Verified HGSS commands

Search for an existing use first:

```sh
rg -n "<command>" armips
```

Open the complete surrounding block rather than copying an isolated line.
Confirm what prepares each parameter, whether the command pauses script
execution, and what commands follow it.

If the repository has no working use, consult the DSPRE command database and
verify that the entry specifically covers HeartGold/SoulSilver. Do not copy a
Generation III, pokeemerald, or Platinum-only example.

## Verified identifiers

Search every identifier independently:

```sh
rg -n "<variable-or-flag>" armips data documentation include src
rg -n "<message-or-script-id>" armips data
```

For NPC or object IDs, inspect the target map's event data. A matching numeric
ID on another map is not evidence.

## Minimal pattern

Copy the known-good HGSS syntax and parameter order, changing only values that
were independently verified for the target script.

## Control-flow checklist

- Preserve the existing branch structure where possible.
- Verify waits after messages, movement, fanfares, and fades.
- Verify lock, release, return, `endstd`, and `end` behavior on every path.
- Do not introduce a command merely because its name sounds appropriate.

## Build and manual verification

Use the repository's normal script-aware ROM build instructions. Manually
exercise every changed branch from an in-game save; assembly success alone does
not validate task ownership, transitions, or script termination.
