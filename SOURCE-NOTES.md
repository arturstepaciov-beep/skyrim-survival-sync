# Reborn Survival Sync — source alpha 0.1

This is a SOURCE PATCH, not an installable mod or compiled Windows release.
The first milestone is shared sleep/wait time and elapsed-game-time accounting,
which are prerequisites for built-in Survival Mode. This does NOT yet establish
that hunger, fatigue, cold or their penalties work correctly in multiplayer.

## Implemented

- Local sleep/wait events send a desired wake-up time to the server.
- An opt-in server setting permits any assigned player to advance shared time.
- Absolute targets avoid adding two overlapping eight-hour sleeps into sixteen hours.
- Duplicate, stale, non-finite, impossible-date and over-24-hour targets are ignored.
- Forward clock jumps also update GameDaysPassed on connected clients, intended
  to keep script timers aware of elapsed time. Connection does not apply historic
  elapsed days as a new time jump. Disconnect clears the pending request.
- Local hunger, cold and fatigue are not overwritten with another player's values.
- Existing animation opcodes and quest changes in the base branch are preserved.

## Important behavior / limits

One player's completed sleep advances the world for everybody on that server.
Awake players do NOT receive a sleep event or automatic rest benefits. There is
no all-player-ready vote yet. Interrupted sleep sends no skip (partial sleep is
not handled). Survival event timing and possible double accounting by the game's
own sleep routines require validation. Cold/weather and remote survival penalties
are not separately synchronized by this patch. Survival Mode must be enabled
manually on both clients, with identical official Survival content installed.
Only private test sessions are appropriate for this source alpha.

New client opcode: both clients AND server must be compiled from this patch.
Do not mix this with the public Reborn release or public servers. Reconnecting
with different saves can still change server date via Reborn's existing join policy.
The base is the upstream DEVELOPMENT branch, not a stable-release fork.
No Ordinator, Weapon Arts, weapons or magic compatibility has been tested.

## Build on Windows

Required: Git, Visual Studio 2022 with Desktop development with C++ and Windows
SDK, xmake 3.1+, Node.js 20 and pnpm 9. Use Developer PowerShell for VS 2022.

Run `./Build-Windows.ps1` from this extracted package. It checks prerequisites,
clones the pinned source, applies the patch, gets submodules, builds, runs the new
packet test, builds the UI and installs output into its own `distrib` directory.
It does not touch Skyrim. This script has not been executed on Windows here.
Dependency downloads and upstream build failures can still need resolution.

Use upstream's build guide to assemble the launcher/client/server/UI output:
https://wiki.tiltedphoques.com/tilted-online/technical-documentation/build-guide

## Configuration after a successful build

In the custom server's config/STServer.ini, under [Gameplay], enable:

```
bEnableSurvivalTimeSync=true
bSyncPlayerCalendar=true
```

bEnableSurvivalTimeSync defaults to false. Use the same complete custom build on
both test PCs and the server. Keep the stable installation and saves backed up.
Do not install the .patch into Vortex: it is applied to C++ source by the script.

## Checks actually completed here

Compiled and executed two C++20 standalone test binaries with g++ and undefined
behavior sanitizer. Policy tests cover duplicates, overlapping and out-of-order
sleeps, invalid dates/values, 24-hour bounds, midnight/month/year rollover, and
20/30/60/120/144 FPS inputs. The other suite compiles the actual DateTime.cpp and
checks AddHours, rollover, ordinary time-scaled ticking and repeated targets.
Its Buffer header is a DECLARATION STUB, only for the standalone calendar test.

A real packet round-trip test was added to TPTests, but that test and the complete
client/server have NOT been compiled or run here. No Skyrim in-game test performed.
Passing the standalone tests is not proof of working multiplayer Survival Mode.

## Required two-player game validation

1. New separate test saves, matching base game and official Survival Mode content.
   Enable Survival on both; no additional gameplay mods for this first test.
2. Connect using the custom server; verify date/hour agree and hunger/fatigue
   progress independently during normal play.
3. Player A sleeps eight hours; check both clocks and GameDaysPassed advance
   approximately eight hours. Only A should receive normal sleeping benefits.
4. Both sleep eight hours concurrently: world must advance about eight, not sixteen.
5. Try different overlapping durations, interrupted sleep, midnight and reconnect.
6. A eats while B does not; only A's hunger should improve. Verify HUD stages,
   health/stamina/magicka penalties and fatigue after the sleep skip.
7. Check cold indoors/outdoors and warming near fires independently.
8. Repeat with one client at 30 FPS and the other at 60 FPS. Inspect shared-time logs.

If in-game survival values do not follow the new elapsed-time counter, the next
step is a game-specific Survival script/engine hook. This source alpha doesn't
pretend that hook has already been implemented.

## Provenance / license

Base: https://github.com/tiltedphoques/TiltedEvolution
Commit: 9ce0338522277e0ff0a12e8d2aa09ce67ad41a59
Sleep event plumbing adapted from Sergio Rayo's upstream PR #897:
https://github.com/tiltedphoques/TiltedEvolution/pull/897
Commit: 87b8d7effbcbdc45e0922965f36009b87e851e3b

Additional work: target-time protocol and validation, precision-aware elapsed-day
accounting, tick reset, tests, and build packaging. GPL-3.0-or-later under the
upstream license, included. Full corresponding base source is available above;
the patch contains all modifications. This is unofficial and not endorsed by
Skyrim Together's maintainers.
