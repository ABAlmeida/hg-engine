---
name: budget-heartless-gold-code
description: Review and plan Pokémon Heartless Gold hg-engine changes against injected-code, ROM archive, save-data, heap, stack, and VRAM limits. Use for implementation plans, code reviews, linker overflows, new hooks, C or assembly additions, large tables, scripts, text, graphics, overlays, or any question about saving bytes or runtime memory.
---

# Budget Heartless Gold Code

Apply the repository's build and test restrictions. This skill adds memory
budgeting; it does not authorize builds, tests, ROM inspection, or emulation.

## Classify the cost first

Identify which budget each change affects:

- C/assembly linked through `src/linker.ld`: fixed overlay-129 injected region.
- C literals, static tables, `.data`, and `.bss`: also consume that region.
- `data/text` and field scripts: message/script NARCs, not injected code.
- Images, palettes, audio, and other assets: ROM size plus load heap/VRAM.
- Save-structure changes: save size, serialization, checksums, and access code.
- Local variables and recursion: stack.
- Dynamic allocations: the owning runtime heap and lifetime.

Do not describe ROM space, injected code, heap, stack, save space, and VRAM as
interchangeable.

## Planning workflow

1. Inspect the relevant call flow, hook sites, objects, and current budget.
2. Compare a data/script solution with an injected-code solution when both are
   behaviorally sound.
3. Estimate recurring runtime cost and one-time memory cost.
4. Prefer the smallest design that remains centralized, testable, and robust.
5. Set a headroom target; do not plan merely to fit by one or two bytes.
6. After changes, compare object/function sizes and review every memory class
   affected. Never claim the build fits without a user-supplied successful
   link result.

## Practical byte-saving rules

1. Track linker headroom after every injected-code feature.
2. Keep dialogue in text archives rather than C string literals.
3. Keep large tables in source-controlled NARCs or data files when practical.
4. Avoid large globals: this linker layout charges `.bss` to the fixed region.
5. Prefer one central hook over several duplicated hooks.
6. Share assembly continuation logic between equivalent hook sites.
7. Centralize validation instead of duplicating it across callers.
8. Reuse established engine tasks and UI modes only when integration stays
   simpler than a custom path.
9. Avoid custom UI overlays unless their benefit justifies mapping, hook, code,
   heap, and verification costs.
10. Review macros and `static inline` helpers for duplicated expansion. A
    shared non-inline helper may be smaller.
11. Measure before micro-optimizing; narrower integer types do not guarantee
    smaller ARM code.
12. Never enlarge a linker region without mapping adjacent runtime memory and
    proving it is unused.
13. Treat `-ffunction-sections` and linker garbage collection as a dedicated
    build-system change. Externally installed hook entrypoints need explicit
    retention.
14. Prefer a linker map or explicit memory-budget check so loss of headroom is
    visible before an overflow.

## Review priorities

Report:

- bytes added or removed by relevant objects and largest functions;
- remaining linker headroom when a successful link result is available;
- static tables, literals, globals, or duplicated trampolines worth moving or
  consolidating;
- heap size, allocation lifetime, failure behavior, and cleanup;
- stack-heavy locals or nested calls;
- archive/VRAM effects of text and assets;
- fragility introduced solely to save space;
- future features likely to consume the same budget.

Do not sacrifice correctness for a small saving without presenting that
trade-off to the user.
