# Shorten a scripted tutorial without running its battle

## Goal

Keep the Route 29 counterpart and Marill introduction while skipping HGSS's
simulated capture battle and its duplicate Poké Ball gift.

## Target and verified command

- Source: `armips/scr_seq/scr_seq_revised_opening.s`
- Member: 225, command offset `0x4FB`
- Verified command: opcode 251, `CatchingTutorial`, from
  `.scratch/pret-pokeheartgold/files/fielddata/script/scr_seq/scr_seq_0225_R29.s`

The patch asserts both `CatchingTutorial` and the following known movement
command before replacing the six bytes with a `goto`. The destination copies
the original cleanup: departure movements, object hiding, hide flags,
`VAR_UNK_408B`, `FLAG_UNK_09A`, `releaseall`, and `end`.

## Minimal pattern

```asm
.org VERIFIED_COMMAND_OFFSET
goto shortened_cleanup

.org ORIGINAL_MEMBER_END
shortened_cleanup:
gender_msgbox VERIFIED_FEMALE_MESSAGE, VERIFIED_MALE_MESSAGE
wait_button
closemsg
// Copy the original cleanup exactly.
releaseall
end
```

## Manual verification

Check both protagonist genders, confirm the grass animation still runs, and
confirm there is no tutorial battle, extra Ball gift, capture-rule mutation,
or second trigger after re-entering Route 29.

