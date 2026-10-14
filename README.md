# Skyrim Together — Survival Sync alpha 0.1

Unofficial development build. Windows compilation and packaging run in Actions.
A successful compiler build does not establish in-game Survival Mode compatibility.

## Download after a successful build

Open Actions → Build Survival Sync Windows → successful run → Artifacts →
SurvivalSync-v0.1-Windows. Extract the outer artifact archive to get the mod ZIP.
Install that inner ZIP with Vortex as a mod. Use the SAME custom client and server
on both PCs; enable official built-in Survival Mode on both test saves.
The bundled server config enables shared survival time and calendar sync.

Implemented: sleep/wait target-time requests, duplicate/overlap protection and
elapsed-game-time corrections. Hunger/fatigue/cold gameplay remains unverified.
No all-player sleep vote; one player's completed sleep advances the whole server.
Interrupted sleep is currently ignored. Keep separate test saves.

The native code is pinned to upstream commit
9ce0338522277e0ff0a12e8d2aa09ce67ad41a59 and modified using the included patch.
See SOURCE-NOTES.md for provenance, limitations and the two-player test checklist.

## Build

The workflow clones the pinned upstream, applies the patch, compiles Windows
client/server, runs the packet test, builds the UI and packages the binaries.
No files on your own PC are changed by the cloud build.
