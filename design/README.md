# Design sheets

The sheets in `sheets/` are the source of truth for Ashina Arena. Code is generated from them
(`tools/gen.py` → `src/generated/sheets.h`); change a sheet before changing code.

## Format

Each sheet is one JSON file:

```json
{
  "sheet": "upgrades",
  "description": "...",
  "columns": {
    "id":        {"type": "id"},
    "effect":    {"type": "enum", "values": ["speffect", "stat"]},
    "sound":     {"type": "ref", "ref": "sounds.id"},
    "notes":     {"type": "string", "optional": true}
  },
  "rows": [
    {"id": "kick", "milestone": "M2", "effect": "speffect", "sound": "upgrade_picked",
     "_unverified": ["effect"]}
  ]
}
```

- One row per thing, one column per property. Every row has `id` and `milestone`.
- A cell that is `null` or missing is **unfilled**.
- A column named in the row's `_unverified` list is filled with our best reading but **not yet checked
  against the real game files**. It still counts as unfinished.
- `ref` columns must name an existing row in another sheet (`sheet.column`). A `ref_list` column holds a
  list of such references.
- `_source` (optional) says where a value came from or where to verify it.

## Milestones

| Milestone | What it delivers |
|---|---|
| M0 | Loader: the proxy DLL Sekiro loads, logging, finding Clone Drone on the PC |
| M1 | Arena: title-screen option, closed arena, endless round director, high score |
| M2 | Upgrades: Clone Drone upgrade pick between rounds, applied to Wolf |
| M3 | Clone Drone audio: commentator lines and arena music from the player's copy |

## Preflight

`python3 tools/preflight.py --milestone M0` lays every sheet over each other for rows up to that milestone
and lists unfilled cells, unverified cells and references that don't resolve. Build only when it is clean.
