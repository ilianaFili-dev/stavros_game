# Ashina Arena

A Sekiro: Shadows Die Twice × Clone Drone in the Danger Zone mashup, made for [Melty](https://melty.gg).

From Sekiro's title screen, pick **Arena**: Wolf is dropped into a closed-off Ashina courtyard and fights
endless, escalating rounds of Sekiro enemies, paced like Clone Drone's arena. Clone Drone's commentator
calls the rounds over its arena music, and between rounds you pick one of Clone Drone's upgrades, which
changes how Wolf fights. Your best round is saved as a high score.

- **Played in:** Sekiro: Shadows Die Twice (Steam, offline, single player).
- **Clone Drone in the Danger Zone** (Steam app 597170) brings its commentator lines, arena music, upgrade
  names and icons and its round pacing, read from the player's own install at run time. Nothing from either
  game ships with the mod. Without Clone Drone installed, the arena still runs and says Clone Drone is missing.
- Single player only. Free, and offline only.

## Status

| Milestone | What | State |
|---|---|---|
| M0 | Loader (`dinput8.dll` proxy), log, finding Clone Drone | Builds; unit-tested on Linux; **not yet run in Sekiro** |
| M1 | Title option, arena, endless rounds, high score | Designed; needs Sekiro's exe and map data |
| M2 | Clone Drone upgrades between rounds | Designed; needs both games' data |
| M3 | Clone Drone commentator and music | Designed; needs Clone Drone's data files |

## How it is built

The design lives in `design/sheets/*.json` (see `design/README.md`); code is generated from it.

```sh
tools/build.sh M0      # preflight → generate → tests → cross-build build/win/dinput8.dll
python3 tools/preflight.py --milestone M3   # everything still open in the full design
```

Needs Python 3, CMake and MinGW-w64 (`g++-mingw-w64-x86-64-posix`).

## Install (what Melty does)

Copy `dinput8.dll` into the Sekiro folder (next to `sekiro.exe`) and start the game. The log is at
`%LOCALAPPDATA%\AshinaArena\logs\latest.log`.

## Credits

See `CREDITS.md`.
