# SurvivalSync v0.8.2

Custom Skyrim Together client and server based on upstream commit 9ce0338522277e0ff0a12e8d2aa09ce67ad41a59. Install matching v0.8.2 client files on both PCs and run the included v0.8.2 server. Keep SurvivalSyncCoop.esp enabled. Retains v0.8.1 housing repairs and earlier features. Preserve config/CoopChest.dat when updating.

## Co-op fixes and HUD

- Shares item/gold grants issued by Papyrus quest and dialogue fragments (QF_/TIF_) with the current party. Tracks source fragment, callsite, item and occurrence to avoid doubling a reward when the same script executes locally and remotely. This does not invent rewards from a completed journal flag or copy unrelated loot. Custom reward systems outside these script paths need separate adapters.
- Party locations update every second, including distant players. Adds exterior native world-map markers, a compass overlay, and projected overhead names for loaded party actors. Interior destinations remain available through the teleport buttons; exterior map markers are hidden when a player enters an interior.
- Adds direct "Teleport to <name>" buttons in F2 using the player's current server location.
- Adds a separate hunger bar driven by the installed official Survival Mode hunger globals, with percentage and stage label. It is hidden when Survival Mode is inactive.
- Restores the fast-travel control disabled by Survival Mode's watcher while retaining hunger, cold and exhaustion updates. Other normal engine travel checks remain in effect.
- Crop harvest requests validate world and distance instead of rejecting a nearby exterior plot across a cell boundary. Mature flora activation is unblocked, native harvest uses default processing, and rejected/unready requests provide feedback.
- Shared chest commits send one revisioned item delta instead of the complete inventory. Open chests reconcile only changed item variants immediately. Snapshot recovery handles missing revisions; the offline view cache is coalesced while server storage is still committed before acknowledgement.
- Replicates SJG Dual Wield Parrying's blockStart/blockStop graph inputs when its plugin is loaded. Reconciles local block intent after the legacy script updates its conditional state, with bounded retries if the animation is cancelled. Both PCs need matching parry/animation mods.
- F2 wait/sleep speed slider ranges from 1x to 10x, defaults to 3x, and retains the 168-hour/seven-day maximum.

Native build, protocol and policy tests are automated. Two-PC gameplay, map-marker save/load behavior, mod compatibility, and physical chest clearance still need in-game testing.

## Housing layout hotfix

Stop the server and update BOTH clients and the server together. The layout binding indices changed, so mixed v0.8/v0.8.1 sessions are incompatible. Preserve config/CoopChest.dat.

- Includes initially enabled control markers, not just initially disabled upgrade objects. Enable-parent chains and inverted enable relationships are reconciled across all supported official homesteads. Child objects no longer overwrite their parent's shared state.
- Known before/after upgrade pairs suppress obsolete groups when the replacement is active, including saved Goldenhills barrel/hay overlaps. Duplicate snapshots cannot undo an unsent local upgrade.
- Moves the Goldenhills network chest off the porch into the approach yard, away from the mapped growing plots. Existing saved gateways are moved and duplicate gateways are consolidated. The shared chest contents remain server-owned.
- Chest clearance is reviewed against official layout records. Ground clearance, modded layouts and in-game co-op behavior still require verification.

## Retained changes

- NPC health uses signed actual damage divided by the attacker's local maximum enemy HP. The server combines each hit once and broadcasts one shared fraction. A 30% hit and a 10% hit leave 60% on both PCs, even with different enemy maximum HP. Player levels and native damage calculations remain unchanged. Essential NPC protection stays native.
- Either player can publish official house ownership, building completion flags, furnishings, construction recipe tokens, and enable states. Remote state is applied directly without replaying purchases or charging gold/materials again. The first connected player's save seeds the session; guests should connect before building. Shared progress remains in each connected player's save.
- Official Hearthfire and installed Creation Club homesteads are indexed by plugin name and local form ID, including Goldenhills upgrades. Missing optional plugins are ignored. Third-party house mods require their own bindings.
- Plant selection uses the normal planter menu. Crop type, growth time and harvested state are shared. The server serializes harvest requests before allowing native crop rewards. Crops mature after one shared day and regrow after three; interrupted/offline play hands unfinished crop timers back to the native scripts.
- Added a shared network chest at the Tundra Homestead exterior entrance when its plugin is installed.
- Native sleep/wait slider supports 168 hours (seven days); the supported AE native wait timer runs three times faster. Shared calendar requests also accept seven days across month/year boundaries. A conflicting wait-speed patch is detected and left in control.

## Installation and testing

Back up saves and the current custom client. Extract the Windows package over the previous custom package on both PCs, preserving the server's config/CoopChest.dat. Run the included server and use matching game/plugin versions. The generated server configuration enables shared survival time.

Native compilation and automated tests are separate from game validation. Test with two PCs: mixed-level melee/spell damage and death; each player buying/upgrading a house; Goldenhills crafting upgrades; planting and simultaneous harvests; Tundra chest access; sleeping 168 hours and cancelling a wait. Chest ground clearance and native Papyrus/Scaleform integration require in-game verification. Farmhand passive pantry production is not a shared inventory in this update.

## Build

Run Build-Windows.ps1 on Windows with Visual Studio C++ tools, xmake 3.1, Node and pnpm 9. It applies the retained v0.6 and v0.7 patches, then world-progress-v08.patch, decompresses the generated binding manifest, compiles the client/server, runs tests and packages the UI and game files. The manifest contains identifiers and field bindings derived from installed official plugins; it contains no Bethesda scripts or game assets.

