# Pokémon Heartless Gold Story Plan

Last updated: 2026-09-08

## Purpose and status

This is the working source of truth for Heartless Gold's player-facing story.
It records the intended plot, character arcs, themes, chapter structure, and
major choices before those ideas are divided into map, dialogue, trainer, and
script implementation work.

The currently implemented opening is defined in `OPENING_SEQUENCE_PLAN.md`.
The new opening premise in this document is a **Draft** that may supersede
parts of that sequence. It does not authorize implementation work or rewrite
the existing plan's technical design and verification record. Differences
between the two are listed explicitly so that no current behavior is mistaken
for accepted future story continuity.

## Planning conventions

Use these labels while developing the story:

- **Decided**: accepted as part of the intended story.
- **Draft**: concrete enough to discuss, but still open to revision.
- **Open**: a question that needs a creative decision.
- **Cut**: considered and deliberately rejected; retain the reason so it is
  not accidentally reintroduced.

For every chapter, distinguish the narrative beat from its eventual technical
implementation. A complete chapter plan should identify:

- where it takes place and when it becomes available;
- the player's immediate objective and motivation;
- the characters present and what each of them wants;
- the conflict, discovery, or choice that changes the situation;
- the consequence that carries into later chapters;
- the gameplay or world-state change that follows; and
- any continuity that a later scene must remember.

Do not assign script archives, flags, variables, message IDs, or object IDs in
this document. Record those in a focused implementation plan after the story
beat is accepted and the relevant game data has been inspected.

## Draft story premise

Pokémon populations across Johto and Kanto are dwindling. Wild Pokémon have
become difficult to find, which is why trainers must use Bait to draw them
out. Team Rocket is responsible for the crisis: its attacks and repeated
attempts to infiltrate the Legendary Pokémon Sanctuary have damaged the
balance that sustains Pokémon populations.

Some Legendary Pokémon had begun to return, and conditions appeared to be
improving. A new attack on the Sanctuary reverses that hope. The Sanctuary is
left more barren than ever, and Lance warns that the wider balance of Pokémon
life is now collapsing.

The Legendary Pokémon Sanctuary replaces the Bug-Catching Contest as a story
location and institution. It exists specifically to shelter Legendary
Pokémon. The balance created by those Legendary Pokémon maintains the balance
of all Pokémon populations; when too many Legendaries are driven away, the
wider population dwindles.

### Decided Sanctuary rules

- The Sanctuary is specifically for Legendary Pokémon.
- The collective balance of its Legendary Pokémon sustains the balance of all
  Pokémon populations.
- Returning missing Legendary Pokémon to the Sanctuary is a recurring side
  storyline throughout the journey.
- The Sanctuary also admits trainers through rare formal-entry permits. A
  sanctioned visit can result in one Legendary Pokémon joining the trainer,
  without changing the player's separate role of returning displaced
  Legendaries encountered in the story.
- The exact list and order of returning Legendary Pokémon remain open until
  the regional chapters are developed.

### Decided capture-conservation rationale

- To avoid further disruption to already dwindling Pokémon populations,
  trainers may capture only the first eligible Pokémon they encounter in each
  area. This prevents repeated hunting from putting sustained pressure on one
  local population.
- A genuinely shiny Pokémon is exempt from the used-area restriction because
  shiny Pokémon still require further study.
- The existing rule is narrower than a universal shiny exemption: the shiny
  Pokémon must not be an exact-species duplicate. The reason for the separate
  duplicate-species restriction remains **Open**.
- The opening must communicate the first-encounter restriction and shiny
  research exception before the player receives a normal capture opportunity.
- The institution or technology that enforces the restriction remains
  **Open**.
- Shiny Bait deliberately creates encounters that qualify for the research
  exception. Why this does not undermine the conservation rule—such as strict
  scarcity or controlled research use—remains **Open**.

### Draft opening sequence

1. The player comes downstairs and finds Cynthia speaking with Mum. Mum
   completes the existing early setup and gives the player's supplies before
   the player leaves with Cynthia.
2. Cynthia takes the player to Professor Elm's laboratory. They encounter the
   gender-selected counterpart, Lyra or Ethan, along the way.
3. At the laboratory, the player chooses a first Pokémon. Elm still gives the
   Healing Kit and completes his other retained opening responsibilities;
   Cynthia gives the tracked Togepi Egg.
4. Afterward, the player overhears Elm and Cynthia discussing the dwindling
   Pokémon population. They explain that Pokémon are now difficult to find
   and that Bait has become necessary.
5. The player learns that Team Rocket has been attacking and trying to
   infiltrate the Legendary Pokémon Sanctuary. Some Legendary Pokémon had
   returned, so the situation had seemed to be improving.
6. The opening conversation mentions that an unnamed trainer is searching for
   Rayquaza. The player-facing dialogue must not identify that trainer as Red.
7. Lance, Steven, and the counterpart enter during the conversation. Lance
   reports a new attack that has left the Sanctuary more barren than ever.
   The balance of Pokémon life is now in genuine danger.
8. During the crisis meeting, Silver runs into the laboratory, takes a
   Pokémon, and attempts to escape.
9. Clair blocks or intercepts Silver at the exit and then joins the assembled
   group. Whether Silver is physically detained, escapes later, or is allowed
   to leave with the Pokémon is **Open**.
10. Lance recognizes Silver's determination and chooses to mentor him. This
   decision must not read as Lance casually condoning the theft.
11. The three young trainers are paired with mentors: Cynthia mentors the
    player, Steven mentors Lyra or Ethan, and Lance mentors Silver.
12. The immediate objective that sends the three protégés onto their journeys
    is **Open**.

### Draft mentorship structure

| Mentor | Protégé | Initial relationship | Possible thematic function | Status |
| --- | --- | --- | --- | --- |
| Cynthia | Player | Cynthia recruits or escorts the player before the crisis is revealed | Understanding history, ecology, and the connections that hold the world in balance | Draft |
| Steven | Lyra or Ethan | Steven selects the counterpart after the Sanctuary report | Investigation, patient observation, and rebuilding rather than merely winning | Draft |
| Lance | Silver | Lance intervenes after Silver steals a Pokémon and recognizes his determination | Turning raw strength and defiance into disciplined protection | Draft |

The thematic functions are development directions, not settled
characterization. The mentors should challenge their protégés differently,
and the three journeys should reveal different parts of the same crisis.

### Story engine created by the premise

The mentorship structure can give the full journey a recurring rhythm:

- the three protégés follow separate leads or assignments;
- they reunite at major milestones to compare discoveries and show how their
  relationships with their mentors are changing;
- Gym Leaders serve as regional authorities, witnesses, protectors, or
  obstacles within the ecological crisis rather than existing only as badge
  checkpoints;
- Team Rocket's movements create the unusual Johto-Kanto route instead of the
  regional crossings feeling arbitrary;
- encounters with displaced Legendary Pokémon create a recurring side path
  in which the player helps them return to the Sanctuary and gradually
  restores the wider balance; and
- the three paths converge as the threat to the Sanctuary and its Legendary
  Pokémon reaches its decisive stage.

This is a **Draft** organizing model. The story can use it without requiring
all three protégés to receive equal screen time at every Gym.

## Established continuity

### Existing implementation baseline

The following is decided and implemented in the current project, but portions
of it may be revised if the new opening premise is accepted:

- New Game enters Professor Oak's introduction through the existing No Info
  Needed path, then preserves character setup, naming, save initialization,
  the shrink sequence, and bedroom entry.
- The normal Lyra/Ethan counterpart selection remains in use. A separate
  forced-female-protagonist idea is still pending and is not assumed here.
- Mum consolidates the early travel setup into her first downstairs scene.
- Professor Elm gives the starter, Healing Kit, and tracked Togepi Egg. The
  player must hatch the Egg, show its Togepi-line Pokémon to Elm, and raise it
  to level 5 before leaving the lab.
- Elm's assistant then supplies the starting Potions, Poké Balls, Poké Bait,
  and Fishing Rod.
- The rival is always named Silver. Silver 1 occurs immediately outside Elm's
  lab after the player completes the Togepi requirements.
- The counterpart's recurring companion is Slakoth. The shortened Route 29
  tutorial explains Poké Bait without staging a capture battle.
- The Cherrygrove guide tour is skipped because the player already has its
  travel unlocks.
- Mr. Pokémon gives Shiny Bait and explains its relationship to the capture
  rules. Professor Oak gives the Pokédex and leaves normally.
- Elm releases the player from the vanilla return-to-lab objective, allowing
  progression toward Violet City.
- The matching counterpart must be defeated in the Route 31-Violet gatehouse
  before giving the Vs. Recorder. Losing does not complete the scene.
- After Falkner, the Violet City assistant gives Shiny Bait instead of a
  duplicate Egg.

The exact ordering, implementation constraints, and outstanding manual checks
remain authoritative in `OPENING_SEQUENCE_PLAN.md`.

### Draft opening integration tasks

These are additions or redesign tasks, not contradictions in the story:

- Extend Mum's downstairs sequence so she completes the retained early setup
  and supplies before the player leaves with Cynthia.
- Preserve the normal gender-selected Lyra/Ethan counterpart and Slakoth
  scenes while adding the encounter with Cynthia on the way to Elm.
- Keep Elm's starter, Healing Kit, phone registration, and other retained
  responsibilities, but move the tracked Togepi Egg gift to Cynthia.
- Preserve the Egg's hatch, show, and level-5 requirements unless the story
  later gives a reason to change them.
- Add the population-crisis discussion and the arrivals of Lance, Steven, the
  counterpart, Silver, and Clair to the lab sequence.
- Redesign Silver's visible theft, Clair's interception, the outcome of the
  stolen Pokémon, and the placement or meaning of Silver 1.
- Define Clair's reason for being in New Bark and establish a thread that pays
  off during the eventual Blackthorn chapter.
- Define why Cynthia, Steven, and Lance mentor novice trainers while retaining
  their own active roles in the crisis.
- Make Lance's mentorship an accountable response to Silver's potential, not
  a reward for theft.

### Decided: gameplay facts the story may need to acknowledge

- Pokémon do not gain experience from battle.
- Eligible fainted party Pokémon are permanently lost after battle. A full
  party wipe recovers the first usable boxed Pokémon, or ends the run when no
  reserve remains.
- Ordinary captures are limited by the established area, duplicate-species,
  Safari Zone, Bug-Catching Contest, and Shiny Bait rules.
- Trainers must be approached and spoken to before battle.
- Trainer victories can grant deterministic first-victory item rewards.

Whether these rules are simply the world's accepted reality or receive an
explicit narrative explanation is **Open**.

## Story identity

| Question | Status | Decision or working notes |
| --- | --- | --- |
| What does “Heartless Gold” mean inside the story? | Open | |
| What is the central premise beyond the challenge rules? | Draft | Pokémon populations are dwindling because Team Rocket's attacks on the Legendary Pokémon Sanctuary are disrupting the balance of Pokémon life. |
| What emotional experience should define the journey? | Open | |
| How dark should the tone become, and what boundaries should it keep? | Open | |
| What themes should the main plot test? | Draft | Balance, stewardship, scarcity, the responsible use of strength, and what people owe the living world are candidate themes. |
| How closely should the plot follow vanilla HeartGold? | Draft | The geography and Gym journey remain recognizable, but the opening, regional order, Bug-Catching Contest, Team Rocket conflict, and major character roles diverge substantially. |
| Is the story primarily about the player, Silver, Team Rocket, or an ensemble? | Draft | The player remains the lead, supported by a three-protégé structure involving the gender-selected counterpart and Silver. |
| What should make this story possible only in the Pokémon world? | Decided premise | The collective balance of the Sanctuary's Legendary Pokémon sustains ordinary Pokémon populations, so Team Rocket's attacks create a wider ecological collapse. |

## Main cast and character arcs

| Character or group | Starting point | Want | Need | Turning points | Ending | Status |
| --- | --- | --- | --- | --- | --- | --- |
| Player character | Finds Cynthia speaking with Mum, receives a first Pokémon from Elm, learns of the crisis, and becomes Cynthia's protégé | | | | | Draft opening; arc open |
| Silver | Takes a Pokémon during the crisis meeting and is intercepted by Clair; Lance sees his determination and chooses to mentor him | | | | | Draft opening; arc open |
| Lyra or Ethan | Meets the player on the way to Elm's lab, joins the crisis meeting, and becomes Steven's protégé | | | | | Draft opening; arc open |
| Cynthia | Arrives at the player's house, escorts the player to Elm, gives the tracked Togepi Egg, understands the population crisis, and mentors the player | | | | | Draft opening; arc open |
| Steven | Arrives during the Sanctuary report and mentors Lyra or Ethan | | | | | Draft opening; arc open |
| Lance | Reports the latest Sanctuary attack and chooses to mentor Silver | | | | | Draft opening; arc open |
| Clair | Intercepts Silver at Elm's exit and joins the crisis meeting | | | | | Draft opening; role open |
| Red | Is mentioned at the start only as an unnamed trainer searching for Rayquaza | | | | Final opponent at Mt. Silver | Draft clue; identity hidden in opening |
| Professor Elm | Gives the player a first Pokémon and discusses the dwindling population with Cynthia | | | | | Draft opening; arc open |
| Mum | Is speaking with Cynthia when the player comes downstairs | | | | | Draft opening; role open |
| Professor Oak | Gives the Pokédex at Mr. Pokémon's house | | | | | Open |
| Mr. Pokémon | Introduces Shiny Bait | | | | | Open |
| Team Rocket | Has repeatedly attacked and attempted to infiltrate the Legendary Pokémon Sanctuary, causing the population crisis | | | | | Draft premise; motive open |
| Johto Gym Leaders | | | | | | Open |
| Legendary Pokémon | Some had returned to the Sanctuary before a new attack reversed the apparent recovery | | | The player helps displaced Legendaries return to the Sanctuary | Their restored balance sustains ordinary Pokémon populations | Decided premise; individual roles open |

Add supporting characters only when they serve a defined story function or
meaningfully change one of these arcs.

## Story replacements and planning tasks

### Current storylines that must be rebuilt

- Rebuild Team Rocket's campaign around its attacks on and attempted
  infiltration of the Legendary Pokémon Sanctuary. This is an actual
  replacement of the current campaign, not merely an added scene.
- Rework Slowpoke Well to serve the new Team Rocket plot. Cynthia will likely
  participate, but her exact role and the purpose of the Rocket operation are
  still open.
- Replace the Bug-Catching Contest's story identity with the Legendary Pokémon
  Sanctuary while retaining a related formal event structure through rare
  permits.
- Rework the Ho-Oh and Lugia storylines so their main-story encounters do not
  end with the player catching them.
- Rework the legendary beasts along similar lines. Suicune's existing story
  requires a more substantial redesign than the others.
- Connect the unnamed trainer's search for Rayquaza to Red's eventual role at
  Mt. Silver.

### Compatible additions requiring design and implementation

- Add Cynthia to Mum's retained opening and let Mum complete the early setup
  before the player leaves.
- Have Cynthia escort the player and meet the gender-selected Lyra/Ethan
  counterpart along the way.
- Move the tracked Togepi Egg gift from Elm to Cynthia while preserving the
  retained hatch, show, and level-5 sequence.
- Add the expanded lab gathering and the three mentor-protégé pairings.
- Let the counterpart retain both the existing Bait tutorial and Violet
  gatehouse battle while also becoming Steven's protégé.
- Introduce Clair and Lance earlier, then develop their new roles through
  later chapters.

### Later progression questions to revisit

- Place the rebuilt Rocket operations, including the Mahogany and Radio Tower
  material, within the interleaved Johto-Kanto progression.
- Define the early S.S. Aqua crossing and the route from its Vermilion arrival
  to the planned Pewter Gym milestone.
- Define how the Power Plant, Copycat Pass, and Magnet Train material supports
  the required return to Johto after Saffron.
- Choose the route from Blackthorn to the second Kanto circuit at Seafoam.
- Redefine the story conditions for Victory Road, the Pokémon League, Mt.
  Silver, and the final encounter with Red.

## Story structure

The current draft deliberately interleaves Johto and Kanto instead of
finishing one region before beginning the other. The player completes six
Johto Gyms, travels to Kanto aboard the S.S. Aqua, completes six Kanto Gyms,
returns to Johto by Magnet Train for its final two Gyms, and then completes
Kanto's final two Gyms before challenging the League.

This is a progression spine, not yet a chapter list. It fixes the rough order
of major destinations without deciding what happens between them, why each
route opens, or where the main plot's turning points fall.

### Draft progression spine

| Order | Milestone | Region or transition | Status |
| ---: | --- | --- | --- |
| 1 | Start | Johto | Draft story; existing implementation baseline |
| 2 | Violet Gym | Johto | Draft |
| 3 | Azalea Gym | Johto | Draft |
| 4 | Goldenrod Gym | Johto | Draft |
| 5 | Ecruteak Gym | Johto | Draft |
| 6 | Cianwood Gym | Johto | Draft |
| 7 | Olivine Gym | Johto | Draft |
| 8 | S.S. Aqua | Johto to Kanto | Draft |
| 9 | Pewter Gym | Kanto | Draft |
| 10 | Cerulean Gym | Kanto | Draft |
| 11 | Vermilion Gym | Kanto | Draft |
| 12 | Celadon Gym | Kanto | Draft |
| 13 | Fuchsia Gym | Kanto | Draft |
| 14 | Saffron Gym | Kanto | Draft |
| 15 | Magnet Train | Kanto to Johto | Draft |
| 16 | Mahogany Gym | Johto | Draft |
| 17 | Blackthorn Gym | Johto | Draft |
| 18 | Seafoam Gym | Kanto | Draft |
| 19 | Viridian Gym | Kanto | Draft |
| 20 | Victory Road | Indigo Plateau approach | Draft |
| 21 | Elite Four | Pokémon League | Draft |
| 22 | Champion | Pokémon League | Draft |
| 23 | Mt. Silver | Finale approach | Draft |
| 24 | Red | Finale | Draft |

### Working structural divisions

| Part | Progression span | Structural purpose | Status |
| --- | --- | --- | --- |
| Prologue | Start to Violet Gym | Reveal the population crisis and Sanctuary attack; establish the player, the counterpart, Silver, their mentors, the opening requirements, and the journey's initial objective | Draft story layered over an existing implemented opening |
| Part I | Azalea Gym to Olivine Gym | First Johto circuit | Draft order; story open |
| Part II | S.S. Aqua to Saffron Gym | First Kanto circuit | Draft order; story open |
| Part III | Magnet Train to Blackthorn Gym | Return to Johto and complete its Gym circuit | Draft order; story open |
| Part IV | Seafoam Gym to Viridian Gym | Return to Kanto and complete all sixteen Gyms | Draft order; story open |
| Part V | Victory Road to Champion | League climax | Draft order; story open |
| Finale | Mt. Silver to Red | Final ascent and ending | Draft order; story open |

### Structural questions created by this order

- What story event makes the S.S. Aqua available after Olivine Gym, and why
  must the player leave Johto before challenging Mahogany and Blackthorn?
- What drives the first Kanto circuit from Pewter through Saffron rather than
  making it a collection of unrelated Gym visits?
- What changes at Saffron that makes returning to Johto by Magnet Train the
  necessary next step?
- What happens during the Mahogany and Blackthorn stretch that resolves the
  reason for the player's return?
- How does the player travel from Blackthorn to Seafoam for the second Kanto
  circuit? This transition is not yet represented by a milestone in the
  progression spine.
- Why do Seafoam and Viridian need to follow the completed Johto circuit?
- Does the Pokémon League require all sixteen badges, or does the story create
  a different reason that Victory Road only opens after Viridian?
- What does the Champion resolve, and what remains deliberately unresolved so
  that Mt. Silver and Red form a necessary finale rather than an epilogue?

## Chapter outline

Create one row for every major chapter after its purpose is understood.

| Chapter | Location | Player objective | Conflict or revelation | Character change | Gameplay consequence | Status |
| --- | --- | --- | --- | --- | --- | --- |
| Crisis at Elm's lab | Player's house, New Bark Town, and Elm's lab | Receive a first Pokémon and learn why Cynthia has come | The group reveals the Sanctuary crisis and mentions an unnamed trainer searching for Rayquaza; Silver takes a Pokémon and Clair intercepts him | Cynthia, Steven, and Lance take the player, the counterpart, and Silver as their respective protégés | Establishes the crisis, the three central young trainers, and the premise behind scarce encounters and Bait | Draft |
| Opening journey | New Bark Town to Violet gatehouse | Complete the retained opening requirements and act on the mentor's first objective | The exact objective and Silver 1 placement need revision around the new lab scene | Begins to define how the three protégés differ | Unlocks normal Violet progression and the Vs. Recorder if those existing beats are retained | Open revision |
| | | | | | | Open |

## Recurring story threads

Track promises, mysteries, and relationships here so that setup and payoff do
not drift apart.

| Thread | Setup | Development | Payoff | Status |
| --- | --- | --- | --- | --- |
| The player's bond with the tracked Togepi-line Pokémon | Elm requires the Egg to hatch and reach level 5 | | | Open after setup |
| The three protégés | Cynthia mentors the player, Steven mentors Lyra or Ethan, and Lance mentors Silver | Their separate assignments reveal different aspects of the crisis and change each mentor relationship | Their paths converge against the cause of the imbalance | Draft |
| Silver's theft, rivalry, and worldview | Silver takes a Pokémon during the lab crisis and Lance chooses to mentor him | Lance must turn Silver's determination toward protection without excusing his actions | | Draft setup; development open |
| The counterpart and Slakoth | Lyra or Ethan meets the player en route to Elm and retains Slakoth as a companion | | | Draft setup; development open |
| Collapse of Pokémon populations | Elm and Cynthia explain the scarcity; Lance reports that the Sanctuary is more barren than ever | Each region should show distinct evidence and consequences of the collapse | Restoration, transformation, or failure of the balance | Draft |
| The Legendary Pokémon Sanctuary | Team Rocket has repeatedly attacked the refuge that maintains Legendary balance | The player helps displaced Legendary Pokémon return | The restored Legendaries repair the balance of ordinary Pokémon populations | Decided function; progression open |
| The cost of permanent loss | The permanent-death rules are active from a new save | | | Open |
| Team Rocket's campaign | Its attacks are blamed for the population crisis | The player uncovers what Team Rocket is doing and why it spans both regions | | Draft setup; motive and payoff open |
| The return of Legendary Pokémon | Some Legendary Pokémon had returned to the Sanctuary and conditions appeared to improve before the latest attack | The player finds and helps displaced Legendaries return to the Sanctuary | Their collective return restores the balance that sustains ordinary Pokémon | Decided side storyline; encounter order open |
| The unnamed Rayquaza seeker | The opening mentions a trainer searching for Rayquaza without naming him | Later clues can connect his search to the Legendary crisis and Mt. Silver | The trainer is ultimately revealed to be Red | Draft |

## World and continuity questions

| Question | Status | Decision or working notes |
| --- | --- | --- |
| What does the wider world understand about permanent loss? | Open | |
| Why does battling not grant experience? | Open | |
| Who created or controls Poké Bait and Shiny Bait? | Open | Mr. Pokémon currently introduces Shiny Bait, but its origin is undecided. |
| Why are wild Pokémon populations dwindling? | Decided | The balance of the Sanctuary's Legendary Pokémon maintains the balance of all Pokémon. Team Rocket's attacks have driven Legendaries away and destabilized ordinary populations. |
| What is the Legendary Pokémon Sanctuary and how does it replace the Bug-Catching Contest? | Decided core; details open | The former Contest becomes a Sanctuary specifically for Legendary Pokémon. Rare Sanctuary Permits grant one formal visit where one Legendary may ultimately join the trainer. Its custodians, physical organization, and public history remain open. |
| What does it mean that some Legendary Pokémon have “returned”? | Decided | They have returned to the Sanctuary, improving the collective balance that sustains ordinary Pokémon. |
| Why is it safe for a trainer to keep one Legendary after a sanctioned Sanctuary visit? | Open | The answer must preserve the premise that the Sanctuary's Legendary balance sustains all Pokémon populations. |
| Who issues rare Sanctuary Permits, and what earns one? | Open | Permit scarcity is the primary restriction on obtaining a Sanctuary Legendary. |
| Who is the unnamed trainer searching for Rayquaza? | Draft | He is Red, but the opening must not identify him by name. The timing of the eventual reveal remains open. |
| Why does Team Rocket want to infiltrate the Sanctuary? | Open | Its concrete objective must be strong enough to drive the full cross-region plot. |
| Why is only the first eligible encounter in each area available for capture? | Decided | Limiting trainers to one opportunity prevents repeated hunting from further disrupting a local population. |
| Why can a shiny Pokémon bypass an area's used opportunity? | Decided | Shiny Pokémon remain insufficiently understood and are exempt so that they can be studied further. The existing exact-species duplicate restriction still applies. |
| Why are exact-species duplicates forbidden? | Open | The conservation rationale for the area limit does not yet explain why previously caught species are prohibited everywhere. |
| Who or what enforces the capture restrictions? | Open | Decide whether this is a League or Sanctuary rule, Pokédex or Poké Ball technology, the mentors' code, or another mechanism. |
| Why does Shiny Bait qualify for the research exception without undermining conservation? | Open | A deliberately generated shiny encounter could become a loophole unless access or use has an in-world constraint. |
| How do Gym Leaders and ordinary trainers fit the revised rules? | Open | |
| Why are Cynthia, Steven, Lance, and Clair all drawn to New Bark and Elm's lab at this moment? | Open | Their arrivals need a causal chain rather than coincidence. |
| Why do the three champions or former champions entrust vital work to novice trainers? | Open | The protégés need unique access, suitability, or responsibilities so the adults cannot simply solve the crisis themselves. |
| What happens after Clair intercepts Silver, and why does Silver retain the stolen Pokémon? | Open | Lance's mentorship must impose responsibility or consequence rather than reward theft. |
| What is Clair's continuing role after the opening? | Open | Her early introduction should pay off before or during the eventual Blackthorn chapter. |
| How do Ho-Oh, Lugia, and the legendary beasts relate to the main conflict? | Open | |
| What carries the story from Johto into the first Kanto circuit aboard the S.S. Aqua? | Open | |
| What requires the Magnet Train return to Johto after Saffron? | Open | |
| What carries the story from Blackthorn into the second Kanto circuit at Seafoam? | Open | |
| What makes the final Mt. Silver encounter a thematic ending? | Open | |

## Scene card template

Use this template when a chapter is ready to break into scenes:

### Scene: title

- **Status:** Open
- **Location and time:**
- **Participants:**
- **Player objective:**
- **Opening state:**
- **Character wants:**
- **Conflict:**
- **Beat sequence:**
- **Choice or revelation:**
- **Closing state:**
- **Required continuity:**
- **Gameplay consequence:**
- **Dialogue notes:**
- **Implementation follow-up:**

## Priority decisions

Resolve these before drafting the post-Violet chapters:

1. Define Team Rocket's objective, method, and reason for attacking the
   Sanctuary, and how that conflict drives the journey through both regions.
2. Decide the Sanctuary's custodians, layout, history, and starting roster of
   returned and missing Legendary Pokémon.
3. Place the recurring Legendary encounters across the progression spine and
   decide what visible changes each return causes at the Sanctuary and in the
   wider world.
4. Define Red's reason for seeking Rayquaza, how the player follows that
   unnamed thread, and when his identity becomes clear.
5. Decide what happens when Clair intercepts Silver and why Lance responds
   with accountable mentorship rather than simple arrest or forgiveness.
6. Give Cynthia, Steven, Lance, and Clair non-coincidental reasons to converge
   on Elm's lab and explain why the three protégés are necessary.
7. Define the first assignment, lead, or promise that sends the player, the
   counterpart, and Silver into the wider story.
8. Integrate Cynthia's escort, the counterpart's role, the crisis meeting, and
   Silver's theft with the retained opening requirements and scenes.
9. Decide how much of the no-EXP and permanent-death framework receives an
   in-world explanation, then define the remaining
   duplicate-species, capture-enforcement, and Shiny Bait rationales.
10. Define the player's dramatic role, the complete counterpart and Silver
    arcs, and how each mentor relationship changes.
11. Choose the target tone, themes, limits on darker material, and the meaning
   of the title.
12. Define the S.S. Aqua, Magnet Train, and post-Blackthorn transitions that
    bind the interleaved regional structure together.
13. Define the League climax and the final Mt. Silver and Red resolution.

## Decision log

| Date | Decision | Reason | Affected chapters |
| --- | --- | --- | --- |
| 2026-09-04 | Preserve the gender-selected Lyra/Ethan counterpart. Steven mentors whichever counterpart appears. | Retain compatibility with either player character while adding the new mentorship story. | Opening and counterpart arc |
| 2026-09-04 | Have Cynthia give the tracked Togepi Egg while Elm retains his other opening gifts and responsibilities. | Integrate Cynthia directly into the player's established opening progression. | Opening |
| 2026-09-04 | Allow formal entry to the Legendary Pokémon Sanctuary through rare Sanctuary Permits. A sanctioned visit can result in one Legendary Pokémon joining the trainer. | Preserve a restricted way to obtain Legendaries while separating sanctioned Sanctuary access from the storyline about returning displaced Legendaries. | Sanctuary and Legendary availability |
| 2026-09-04 | Limit trainers to the first eligible encounter in each area to protect dwindling local populations from repeated hunting. Let non-duplicate shiny Pokémon bypass the used-area restriction because they require further study. | Give the area limit and shiny clause a direct conservation purpose within the population crisis. | Opening and all capture areas |
| 2026-09-04 | Make the Sanctuary exclusively a refuge for Legendary Pokémon. Their collective balance sustains all Pokémon populations, and returning displaced Legendaries is a recurring side storyline. | Make restoration of the Legendary population the direct means of repairing the wider ecological crisis. | Opening, Legendary encounters, and Sanctuary storyline |
| 2026-09-04 | Mention at the start that an unnamed trainer is searching for Rayquaza. Internally this trainer is Red, but the opening does not identify him by name. | Seed Red and Rayquaza as a long-running mystery that can connect the opening to the Mt. Silver finale. | Opening through finale |
| 2026-09-04 | Use the population collapse, Team Rocket's attacks on the Legendary Pokémon Sanctuary, and the Cynthia/player, Steven/counterpart, and Lance/Silver mentorships as the current draft premise. | Connect the scarcity and Bait mechanics to a character-driven story that can span Johto and Kanto while preserving Lyra/Ethan selection. | Opening and entire journey |
| 2026-09-04 | Use the current draft progression of six Johto Gyms, six Kanto Gyms, the final two Johto Gyms, the final two Kanto Gyms, the League, Mt. Silver, and Red. Use the S.S. Aqua for the first Kanto crossing and the Magnet Train for the return to Johto. | Establish the rough progression spine before writing the plot that connects its milestones. | Entire journey |
| 2026-09-04 | Create a dedicated story plan while retaining the revised opening's established continuity. | Separate narrative development from technical feature and script plans. | All |

## Related plans

- `PROJECT_PLAN.md`: overall feature status and current project decisions.
- `OPENING_SEQUENCE_PLAN.md`: authoritative revised-opening design and
  implementation record.
- `PERMANENT_DEATH_PLAN.md`: fainting, wipe recovery, and no-reserve ending
  behavior.
- `CAPTURE_RULES_PLAN.md`: capture eligibility and special-area rules.
- `POKEMON_TRAINERS.md`: trainer-party content.
- `POKEMON_ENCOUNTERS.md`: wild encounter content.
