# Convert a special mart to standard dialogue

## Goal

Keep an existing HGSS special mart and inventory index while removing a
shop-specific dialogue mode that no longer matches the replacement inventory.

## Target

- Script archive/source: member 94 of `a/0/1/2`, patched by
  `armips/scr_seq/scr_seq_goldenrod_berry_shop.s`.
- Script entry: the Goldenrod Underground herbal merchant.
- Trigger or caller: talking to the existing merchant NPC.

## Verified HGSS commands

- `SetFlag flag`: opcode 30 in `armips/include/scriptmacros.s`.
- `ClearFlag flag`: opcode 31 in `armips/include/scriptmacros.s`.
- `CallStd std_special_mart`: retained unchanged from the known-good pristine
  script in `.scratch/pokeheartgold-reference/files/fielddata/script/scr_seq/`.

The patch replaces only the `SetFlag` opcode for the bitter-mart mode with the
same-size `ClearFlag` opcode. Its existing flag operand and later cleanup
command remain unchanged.

## Verified identifiers

- Map/script member: Goldenrod Underground `D37R0102`, member 94.
- Command offset: `0x1A1`, verified against the pristine member and the
  disassembled reference script.
- Flag operand: the existing special-mart bitter-dialogue flag; its numeric
  value is preserved rather than invented or reassigned.
- Inventory selector: existing special-mart index 8, unchanged.
- Message bank: member 117. Message 0 supplies the merchant's new Berry Shop
  introduction; all other messages in the bank are preserved.
- NPC/object IDs, trainer IDs, map IDs, event IDs, and new variables: none.

## Minimal pattern

```asm
.if readu16("build/a012/2_member", command_offset) != 30 && readu16("build/a012/2_member", command_offset) != 31
    .error "Special-mart flag command has an unexpected opcode"
.endif

.open "build/a012/2_member", 0
.org command_offset
.halfword 31 // ClearFlag
.close
```

Accepting both the pristine and patched opcodes makes deliberate script-archive
rebuilds idempotent while still rejecting an unexpected upstream layout.

## Control-flow checklist

- The NPC's lock, facing, initial message, shop call, release, and end remain
  unchanged.
- The existing special-mart inventory index remains unchanged.
- Cancelling and returning from the shop use the ordinary special-mart messages
  instead of dialogue about bitter medicine.
- No new branch, task, fade, variable, flag value, or asynchronous wait is
  introduced.

## Build and manual verification

Build from MSYS2 UCRT64 with:

```sh
make quick-rom -j$(nproc)
```

Talk to the Goldenrod Underground merchant and verify the Berry Shop
introduction, inventory, buying flow, cancellation dialogue, and return from
the shop. Confirm that no bitter-medicine dialogue remains.
