# HGSS script how-to template

## Goal

State the single reusable scripting behavior.

## Target

- Script archive/source:
- Script label or entry:
- Trigger or caller:

## Verified HGSS commands

For every command, record:

- Command and parameters:
- Known-good repository use:
- DSPRE HGSS database entry, if repository evidence was unavailable:
- Parameter order and meaning:

Do not include an unverified command.

## Verified identifiers

Record the source for every relevant:

- variable or flag;
- message ID;
- trainer ID;
- map or event ID;
- NPC or object ID;
- item or other content ID.

Write `Not used` for categories that do not apply.

## Minimal pattern

Show only the smallest reusable script fragment. Copy syntax and parameter
ordering from the verified HGSS example.

## Control-flow checklist

- All branches terminate or return to an intentional continuation.
- `lockall`/`releaseall` usage is balanced.
- Messages have the required wait and close behavior.
- Fades have the required wait behavior.
- No active script or task is replaced unexpectedly.
- The caller receives the result or state it expects.

## Build and manual verification

Record the narrow build command and focused in-game checks. A successful build
does not prove runtime script behavior.
