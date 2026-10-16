Survival Sync v0.5 - 144 FPS PHYSICS TEST
=========================================

This package retains the already tested v0.5 client/server/UI binaries, including
Hearthfire synchronization, autosync countdown and running/jumping changes.
It adds an optional SSE Display Tweaks profile. No new native Havok patch or
third-party DLL is included. 144 FPS co-op physics has NOT been verified in game.

INSTALL ON BOTH PLAYER PCs
1. Install SSE Display Tweaks and the SKSE / Address Library requirements that
   match your Skyrim executable. Get the plugin from its author:
   https://www.nexusmods.com/skyrimspecialedition/mods/34705
2. Keep your existing server STServer.ini when updating Survival Sync.
3. Close Skyrim. From this folder, run in PowerShell:
   .\Apply-144FPS.ps1 -GameDirectory 'C:\Your\Skyrim Special Edition'
4. With MO2/Vortex, apply the settings to the active winning mod's plugin folder,
   for example by supplying -PluginDirectory 'C:\Mods\SSE Display Tweaks\SKSE\Plugins'.
   You must deploy/enable that mod in your manager. Virtual files invisible to the
   script must be edited in the manager instead: merge Profile.ini settings into
   the active SSEDisplayTweaks_custom.ini, preserving unrelated settings.
5. Start Skyrim Together through your normal SKSE-compatible setup. Check
   Documents\My Games\Skyrim Special Edition\SKSE\SSEDisplayTweaks.log
   (GOG uses its corresponding game folder) to confirm the plugin actually loads.
   Merely copying an INI does not fix physics if its DLL is not loaded.

WHAT CHANGES
Gameplay cap: 144 FPS.
Havok: enabled, dynamic timestep scaling, range 60-144 FPS, physics damage patch.
Menu/loading cap: 60 FPS, keeping the loading cap for two seconds around loading.
Display mode, resolution, VSync, server tick rate and 15 Hz item updates are unchanged.
This cannot guarantee your hardware reaches 144 FPS or bypass other FPS caps.

TEST
Start both players at 144 FPS. Drop/drag objects, push clutter off tables and
observe ragdolls, indoors and outdoors. Then test one player at 60 FPS and the
other at 144 by changing that PC's custom Render/FramerateLimit only.
Leave Havok MaximumFramerate=144. Include FPS drops while testing.
Report flying objects, unusual collision damage, jitter and position differences.

UNDO
Close Skyrim. Restore the timestamped .bak copy of SSEDisplayTweaks_custom.ini.
If no custom INI existed before setup, remove the new custom INI.
Your original SSEDisplayTweaks.ini is never modified by the setup script.

BUILD IDENTITY
BuildInfo.json identifies the retained native binaries and their original build.
ProfileBuildInfo.json identifies this timestamped configuration package, its
source artifact and checksums. The in-game version remains the native v0.5 ID.
