---
name: import-heartless-gold-content-drafts
description: Apply the user's editable Pokemon Trainers.txt or Pokemon Encounters.txt draft to hg-engine source data and trackers. Use only when the user asks to import or apply one of those parent-directory drafts.
---

# Import Heartless Gold Content Drafts

Treat `../Pokemon Trainers.txt` and `../Pokemon Encounters.txt` as user-owned
authoring drafts. Read the requested draft, but do not delete, replace, rename,
or normalize it.

Verify each imported trainer, species, map, item, and other identifier against
authoritative repository source. Transfer only the requested, verified content
into source-controlled game data and the corresponding Markdown trackers.

For trainer-party or encounter imports, also apply
`../maintain-pokemon-availability/SKILL.md`. For first-victory reward changes,
also apply `../maintain-trainer-reward-trackers/SKILL.md`. Preserve unrelated
draft and working-tree content, and report any ambiguous or unverifiable entry
instead of inventing a mapping.
