# MODLOG — Ashina Arena

## 2026-10-08 — design and M0

**Melty findings**
- `sekiro`: engine fromsoftware, exe `sekiro.exe`, no loader Melty installs ("only mashups that work without
  one (plain game files)"). Selling: review; mods must stay offline and free. 0 mashups on Melty.
- `custom-clone-drone-in-the-danger-zone` (Steam 597170): not in the catalog → recipe role `secondary`;
  the mod finds its folder itself.
- No existing mashups for either game.
- one_click_check on a recipe-only draft (mode installed, `dinput8.dll` + files mapped to `{game}`, launch
  game): **oneClick yes, publishable**. Must re-run with the real entries.

**Route:** native hooks via our own `dinput8.dll` proxy + MinHook (BSD-2, bundled).
- ModEngine2 (MIT) lists Sekiro as unsupported (README: "[ ] Sekiro").
- ModEngine 1 (katalash/ModEngine, the usual Sekiro loader) has no license file → can't redistribute.
- So the mod is self-loading. Loose-file overrides (params/events), if M1 needs them, must be our own hook.

**Open (from preflight M3, 83 cells):** Sekiro hook signatures (exe 1.06), arena map id/positions, enemy
NpcParam/chr ids, SpEffect ids for upgrades, Clone Drone audio clip names/container/codec, upgrade keys and
icons, where Clone Drone keeps its arena pacing. All need the games' files on the player's PC.

**Next:** run M0 in Sekiro on the user's PC (log should show "loaded" and the Clone Drone folder), then recon
sekiro.exe and Clone Drone's `_Data` folder to fill the M1 sheets.
