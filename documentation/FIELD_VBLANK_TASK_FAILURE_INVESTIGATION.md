# Intermittent Field VBlank Task Failure Investigation

Last updated: 2026-08-23

## Status

Failure mechanism established. A guarded, source-controlled candidate fix is
enabled but has not been built or verified. The diagnostic setup remains
available for confirming its behavior and identifying changes that affect how
often the race occurs.

A same-frame burst of field-model texture uploads fills the fixed 32-entry
VBlank task queue. A later field-lifecycle allocation consequently returns
null, and field shutdown eventually passes that missing task pointer to
`DestroySysTask`.

## Observed symptoms

- After catching an Unown in the Ruins of Alph, overworld object graphics
  became corrupted while the map itself remained visible.
- Opening the Pokémon menu afterward caused a data abort and apparent
  softlock.
- Similar intermittent object-graphics corruption was previously observed
  during a Day Care cutscene, although that occurrence was not captured in
  the debugger and is not yet proven to share the same root cause.
- Repeating either sequence is not known to reproduce the issue reliably.

## Confirmed debugger evidence

The following addresses describe the current Heartless Gold build and the
unmodified HeartGold field overlay involved in the failure. Reconfirm them if
an upstream update changes the base executable or overlays.

1. The ARM9 entered the data-abort handler at `0xFFFF0104`.
2. Abort-mode LR was `0x0201F960`. The faulting Thumb instruction is at
   approximately `0x0201F958`:

   ```asm
   ldr r0, [r4, #20]
   ```

   `r4` was zero, so this dereferenced address `0x00000014`.
3. User-mode LR was `0x021FA919`, immediately after a field-overlay call to
   `DestroySysTask`.
4. The field cleanup routine beginning at `0x021FA8F8` destroys four task
   pointers stored at manager offsets `+0x18`, `+0x1C`, `+0x20`, and `+0x24`.
5. At the captured failure those values were:

   | Offset | Value |
   | --- | --- |
   | `+0x18` | `0x023BBB98` |
   | `+0x1C` | `0x023BBAF0` |
   | `+0x20` | `0x00000000` |
   | `+0x24` | `0x023BD254` |

6. The missing `+0x20` task is assigned immediately after
   `SysTask_CreateOnVBlankQueue` returns at `0x021FA8D4`. A null return is
   therefore the direct precursor to the later crash.
7. A subsequent live capture stopped before allocation with queue capacity 32
   and active count 30. Of those tasks, 29 used callback `0x02069701`, priority
   `0xFFFF`, and distinct data records that were commonly spaced `0x14` bytes
   apart.
8. Callback `0x02069700` performs its update and then calls
   `DestroySysTask` at `0x0200E390`. These are therefore one-shot tasks queued
   faster than VBlank can drain them, rather than 29 confirmed persistent
   task leaks.
9. Further allocations raised the active count to 31 and then 32. With all 32
   slots occupied, another request for callback `0x02069701` returned zero
   from the allocator at `0x0201F7E4`. Queue exhaustion is therefore confirmed
   as the direct allocation failure.
10. The task requested by the captured failed allocation used data pointer
    `0x0232115C`, priority `0xFFFF`, and returned to the wrapper at
    `0x0201F8E2` with `r0 = 0`.
11. Static inspection identifies `0x02069714` as the producer of callback
    `0x02069701`. Each call schedules one 20-byte field-model resource record
    for texture allocation, texture upload, model binding, and self-destruction
    during VBlank.
12. The records' 20-byte size explains the common `0x14` spacing observed in
    the debugger. The records are owned by their field resource managers and
    are freed only through the normal model-resource teardown path.
13. Overlay helper `0x021F197C` is a separate paired operation. It creates
    callback `0x021F19B5` on the VBlank queue and a companion callback on the
    VWait queue. It accounts for the one different callback in the captured
    queue, not the 29 repeated `0x02069701` callbacks.
14. Resource-load helper `0x021F19F4` calls `0x02069714` once for each model
    resource loaded. A transition that creates many field resources before the
    next VBlank therefore produces the captured burst without a persistent
    task leak.
15. Breakpoints on Unown form generation and follower graphics lookup did not
   fire before the Pokémon-menu abort. All 28 source-controlled Unown
   overworld assets and their generated indices were statically present and
   internally consistent. Unown data is therefore not the confirmed immediate
   cause.

The breakpoint at `0x021FA8D4` is in overlay-loaded memory. A hit while the Bag
was open occurred long before the captured failure and cannot be interpreted
as the field routine without first confirming that the expected field overlay
is loaded. Do not use that address as an unconditional persistent breakpoint.

## Static queue and transfer model

- System startup permanently allocates four fixed task queues from the low end
  of the main OS arena: 160 main tasks, 32 VBlank tasks, 32 VWait tasks, and
  four print tasks.
- A task queue returns null when its active count reaches its fixed limit. It
  has no built-in deferral, growth, or overflow list.
- The VBlank interrupt directly runs the VBlank queue. The model callback must
  remain there because it allocates texture VRAM, uploads texture data, and
  binds the model set.
- The engine's VRAM transfer manager is a separate per-application facility.
  It does not accept or defer these system-task callbacks and is not a drop-in
  fallback.

## Selected candidate fix

`FIX_FIELD_MODEL_VBLANK_QUEUE_OVERFLOW` retargets the verified field-overlay
Thumb call at `0x021F1A10` from the original model-upload scheduler at
`0x02069714` to a guarded wrapper. Before creating another model-upload task,
the wrapper waits for VBlank whenever 28 of the queue's 32 entries are
occupied. This keeps four entries available for field coordination and
lifecycle work. Retargeting the call avoids overwriting the short original
ARM9 function or its neighbouring code.

The candidate deliberately uses synchronous backpressure rather than another
task queue:

- no resource record is copied or given a new owner;
- no retry task can accumulate across frames;
- the original upload callback, resource pointer, and priority are preserved;
- the normal path is unchanged until the queue reaches its high-water mark;
- an unexpected allocation failure waits and retries instead of allowing the
  field to continue with an unscheduled texture upload.

The implementation consumes injected-overlay code through `field_vblank.o`
but adds no BSS, heap, save, archive, or VRAM allocation. Its exact linked cost
and remaining headroom must be measured by the next explicitly requested
build. Runtime correctness remains unverified until the affected transitions
are manually exercised from an in-game save.

This remains a candidate rather than proof that a particular Heartless Gold
feature created the timing change. Its purpose is to remove the confirmed
queue-capacity race at the bulk producer while the saved reproduction state is
retained for further diagnosis.

## Rejected and alternative fixes

Doubling the VBlank queue to 64 would increase its permanent main-arena
allocation from 1,076 to 2,100 bytes: exactly 1,024 additional bytes. It would
not consume injected-overlay space, but it would move every later low-arena
allocation and reduce runtime arena headroom by 1 KiB. The available final
arena margin is not established, and capacity 64 would still have no defined
failure behavior if a larger burst occurred.

One explored implementation kept the original queue allocation and changed
only the confirmed bulk producer:

1. At most 28 model-upload callbacks may occupy the 32-entry VBlank queue.
   Four entries remain available for field lifecycle and other VBlank work.
2. Excess model records receive a retry task on the 160-entry main queue.
3. A retry schedules the original texture work for a later VBlank as soon as
   the reserved-capacity rule allows, then destroys itself.
4. If the main queue is independently full, the 32-entry VWait queue provides
   a second retry path. If both retry queues reject the task, the established
   assertion path is used rather than writing through a null task pointer.

That provisional implementation was removed before being built. Its separate
main-queue retry tasks require stronger boundedness and lifecycle guarantees
and are not used by the selected synchronous candidate.

Do not treat a null check around `DestroySysTask` as the root fix. It could
avoid the immediate abort while leaving the field without a required VBlank
task and allowing graphical or state corruption to continue.

## Persistent debugger setup

Configure melonDS with the ARM9 GDB stub on port `3333`, disable JIT, and
connect from `gdb-multiarch`:

```gdb
set pagination off
set osabi none
set architecture armv5te
symbol-file C:/Projects/pokeheartlessgold/hg-engine/build/linked.o
set remote noack-packet off
target remote 127.0.0.1:3333
```

Keep `set remote noack-packet off` before `target remote`. melonDS 1.1 does
not reset its negotiated no-ack state when GDB disconnects, so reconnecting
can otherwise fail during `qSupported`/`vMustReplyEmpty` negotiation with
packet errors and timeouts. If that failure has already occurred, close GDB,
restart melonDS once to clear the retained state, let the game run, and then
connect with the commands above.

Use the fixed ARM9 breakpoint during ordinary play. It stops before the queue
is completely full, allowing the pending tasks and caller to be inspected.

```gdb
delete breakpoints
break *0x0201f7e4 if ($r0 == *(unsigned int *)0x021d1128) && (*(unsigned short *)($r0 + 2) >= 30)
continue
```

Do not keep an unconditional breakpoint at `0x021FA8D4`: overlays reuse that
address, so unrelated applications such as the Bag can trigger it.

### Automatic burst logger

The local ignored helper `.scratch/vblank-task-tracker.gdb` avoids fragile
multiline pasting into GDB. Source it after connecting to the ARM9 stub:

```gdb
source C:/Projects/pokeheartlessgold/hg-engine/.scratch/vblank-task-tracker.gdb
continue
```

For a fresh GDB session, `.scratch/vblank-repro.gdb` performs the complete
architecture, symbol, remote-connection, logging, breakpoint, and continue
sequence. Start melonDS's ARM9 GDB stub first, then run this from MSYS2 UCRT64:

```sh
cd /c/Projects/pokeheartlessgold/hg-engine
gdb-multiarch -x .scratch/vblank-repro.gdb
```

The complete console trace is also written to the ignored local file
`.scratch/vblank-task-log.txt`, so a long burst does not need to be copied from
terminal scrollback.

It logs the higher-level field-model load arguments and every request for
callback `0x02069701`, including the active count, model-resource pointer,
priority, and direct caller. The expected direct caller is the original
scheduler at `0x02069714`; no overlay coordination-record layout should be
inferred from that model-resource pointer. It stops automatically when another
allocation is attempted with all 32 queue entries active.

The pause at `0x020D3F60` immediately after connecting or resetting is not
one of these diagnostic breakpoints; use `continue`.

## Data to capture when a breakpoint hits

Do not reset or continue until the following information has been recorded:

```gdb
info breakpoints
info registers r0 r1 r2 r3 r4 r5 r6 r7 sp lr pc cpsr

set $vq = (unsigned int)*(unsigned int *)0x021d1128
p/x $vq
p/d *(unsigned short *)$vq
p/d *(unsigned short *)($vq + 2)

set $head = (unsigned int)($vq + 4)
set $node = (unsigned int)*(unsigned int *)($head + 8)
set $i = 0

while ($node != $head) && ($node != 0) && ($i < 40)
  printf "task=%#x priority=%u data=%#x func=%#x\n", $node, *(unsigned int *)($node + 12), *(unsigned int *)($node + 16), *(unsigned int *)($node + 20)
  set $node = (unsigned int)*(unsigned int *)($node + 8)
  set $i = $i + 1
end
```

Also record:

- the breakpoint number and address;
- the immediately preceding player actions and transition;
- whether Poké Bait or Shiny Bait initiated the battle;
- whether a Pokémon was caught, defeated, fled from, or failed to bite;
- whether the party changed and whether a new lead follower was selected;
- whether any party Pokémon fainted or a permanent-death notification ran;
- whether the issue followed a save load, map transition, menu, cutscene, or
  battle; and
- whether the session used only an in-game save or involved a save state.

## How to interpret the next capture

- Active count equal to capacity means queue exhaustion. Group the listed
  `func` addresses and identify repeated callbacks to locate the burst owner.
- Active count below capacity with a null allocation points instead to a
  damaged free-task stack or inconsistent queue bookkeeping.
- A high count that falls after a particular transition identifies the
  cleanup boundary that is running too late.
- A steadily rising count across repeated ordinary transitions identifies a
  leak even if the queue has not reached capacity yet.

## Verification still required

1. Select a bounded fix only after reviewing its task ownership, teardown and
   failure behavior; build it only when explicitly requested.
2. Repeat ordinary field transitions, the Ruins of Alph sequence, Day Care,
   Bait encounters, party-menu entry, and battle return from an in-game save.
3. Confirm the candidate prevents model uploads from starving required field
   lifecycle callbacks without permitting deferred work to accumulate.
4. Confirm that field objects render correctly and that menu entry and field
   teardown no longer reach `DestroySysTask(NULL)`.
5. If another full-queue capture occurs, classify its callback mix before
   changing the reserve or extending deferral to a second producer.

## Investigation log

| Date | Result |
| --- | --- |
| 2026-08-22 | Captured data abort after opening the Pokémon menu following an Unown capture. Identified `DestroySysTask(NULL)` and the missing field VBlank task at manager offset `+0x20`. Allocation-time queue state remains outstanding. |
| 2026-08-22 | Captured the VBlank queue rising from 30 to 32 active tasks. At 30 tasks, 29 entries used the self-destroying callback `0x02069701`. A further request for that callback at count 32 returned null, conclusively establishing same-frame queue exhaustion. An earlier `0x021FA8D4` Bag hit was reclassified as an overlay-address false positive. |
| 2026-08-22 | Corrected the initial ownership inference through static source inspection. `0x02069714`, called once per field-model load, creates callback `0x02069701` for a 20-byte model-resource record. `0x021F197C` creates the single different VBlank callback and its VWait companion. |
| 2026-08-22 | Explored producer-specific backpressure, then removed the provisional implementation before build because its per-resource retry tasks needed stronger boundedness and lifecycle guarantees. Rejected a 64-entry queue for now because it would consume 1,024 permanent main-arena bytes without establishing final arena headroom or defining overflow behavior. |
