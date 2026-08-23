# Shorten a scripted tutorial without running its battle

## Goal

Keep the Route 29 counterpart and Slakoth grass demonstration while skipping
HGSS's simulated capture battle and its duplicate Poké Ball gift.

## Target and verified command

- Source: `armips/scr_seq/scr_seq_revised_opening.s`
- Member: 225, command offset `0x4FB`
- Verified command: opcode 251, `CatchingTutorial`, from
  `.scratch/pret-pokeheartgold/files/fielddata/script/scr_seq/scr_seq_0225_R29.s`

The patch asserts both `CatchingTutorial` and the following known movement
command before replacing the six bytes with a `goto`. The destination copies
the original cleanup: departure movements, object hiding, hide flags,
`VAR_UNK_408B`, `FLAG_UNK_09A`, `releaseall`, and `end`.

The existing pre-animation `GenderMsgBox` uses local messages 0/1, now the
Poké Bait explanation. Both messages end in `\r` so the animation cannot begin
until the player acknowledges the final page. For the Lyra path, the original
helper at `_0559`
already runs the retained grass demonstration and prints local message 2. Keep
that message as only `"...Just like that."`; printing another message after
the branch would duplicate the response. The original helper put `CloseMsg`
directly after that message, so it must also be redirected through a small
`wait_button` / `closemsg` continuation. That continuation replays the
displaced `scrcmd_602 0` before returning to offset `0x590`; otherwise the
follower movement state would change. The Ethan path does not call `_0559`, so
the shortened continuation uses local messages 21/22 for its gendered
response. These IDs belong to member 225's message bank,
`data/text/373.txt`.

## Minimal pattern

```asm
.org VERIFIED_COMMAND_OFFSET
goto shortened_cleanup

.org ORIGINAL_MEMBER_END
shortened_cleanup:
compare VAR_TEMP_x4002, 0
goto_if_eq departure_cleanup
gender_msgbox VERIFIED_FEMALE_MESSAGE, VERIFIED_MALE_MESSAGE
wait_button
closemsg
departure_cleanup:
// Copy the original cleanup exactly.
releaseall
end
```

`VAR_TEMP_x4002` is populated by the original `GetPlayerGender` command before
the patched branch. The original script verifies the same value with
`Compare VAR_TEMP_x4002, 0` and `CallIfEq _0559`. The continuation repeats
that verified comparison solely to avoid printing a second response after the
Lyra-only helper returns. `compare` and `goto_if_eq` use the same parameter
ordering as the existing revised-opening branches in
`armips/scr_seq/scr_seq_revised_opening.s`.

## Manual verification

Check both protagonist genders, confirm the grass animation still runs, and
confirm the opening explanation and `"...Just like that."` each remain visible
until input is received. Also confirm there is no tutorial battle, extra Ball
gift, capture-rule mutation, or second trigger after re-entering Route 29.
