# Coordinated Doubles Trainer AI Plan

Last updated: 2026-08-09

## Status

Deferred and optional. Do not implement as part of the fair-information
trainer AI plan unless the project explicitly reactivates this work.

## Boundary

The generic expert layer may run independently for each trainer battler in a
double or multi battle. It must preserve legal move, target, and switch choices,
but no doubles-specific quality guarantee is part of the current AI project.

This deferred plan would add:

- paired action and target evaluation for both allied active Pokémon;
- spread-move value, friendly-fire, Wide Guard, redirection, and protection;
- ally support such as Helping Hand, screens, speed control, healing, and
  ability combinations;
- coordinated focus fire and avoidance of redundant KOs or duplicated status;
- shared reserve-slot arbitration and coordinated switching;
- doubles-specific player-action prediction and bounded joint search; and
- doubles performance, memory, and manual-verification budgets.

## Reactivation requirements

Before implementation, review the completed singles architecture, measure its
remaining battle-extension headroom and decision time, define the desired
doubles strength, and decide whether multi battles with an NPC partner share
the same behaviour. Produce a separate detailed implementation plan from those
measurements rather than extending the singles plan opportunistically.
