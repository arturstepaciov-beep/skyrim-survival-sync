# SurvivalSync v0.8.8

Custom Skyrim Together client and server based on upstream commit 9ce0338522277e0ff0a12e8d2aa09ce67ad41a59. Install matching v0.8.8 client files on both PCs and run the included v0.8.8 server. Keep SurvivalSyncCoop.esp enabled. Retains v0.8.1 housing repairs and earlier features. Preserve config/CoopChest.dat when updating.

## Player-crafted items and shared sleep

- Player-created potions and poisons transmit their full effect definitions, magnitudes, areas, durations, names, flags, weight and value. Receiving clients reconstruct the generated alchemy form with the native created-object manager, instead of interpreting another player's local FF form ID.
- Generated potion and enchantment identities are based on their content, so round trips and stacking do not depend on which PC created the form. Exact crafted duration/strength is preserved, without substituting a vanilla potion or recalculating with the recipient's skills.
- Shared inventories, ordinary chests, merchant inventory changes, co-op chest transfers and floor copies use the same capture/resolve path. Custom enchantment effects and smithing temper factors are retained.
- Co-op chest persistence writes version 2 with generated definitions and plugin-name remapping, and continues reading existing version 1 files. Preserve config/CoopChest.dat.
- Any completed positive-duration sleep grants all current party members full rest. Interrupted sleep also grants rest if some game time actually elapsed; cancelling before sleep or waiting does not. Survival's fatigue global is cleared and its own DecreaseExhaustion script refreshes stages and magicka penalties, with a delayed second pass for asynchronous sleep scripts.
- Matching v0.8.8 clients/server required. Crafted-item gameplay, merchant buyback, floor pickup and shared rest still need two-PC verification.

## Hand equipment HUD and enemy weapon repair

- Separate Left hand and Right hand equipment names above the hunger bar. Reads native hand objects and spells, shows Empty when unequipped, updates ten times per second.
- The current AI owner publishes authoritative hand IDs, equipment and ownership epoch on changes, plus a five-second heartbeat. Receivers retain the snapshot through unloaded 3D and retry only on remote, living, loaded actors. Stale ownership snapshots are ignored.
- Missing physical hand items are restored from the owner's equipment data before native equip; existing counts are respected, including identical dual-wield copies and single two-handed weapons. Only hand items are repaired, without rebuilding the full inventory.
- New 3D receives hand equipment again. Missing plugins are left unresolved rather than clearing local hands. Existing animation and weapon draw synchronization remain in place.
- Matching v0.8.8 client/server required: equipment packets gained hand snapshots. Native gameplay, modded weapon meshes and both players' enemy visuals still need two-PC verification.

## Map fast-travel hotfix

- Connected co-op players bypass both native map travel eligibility gates (AE 40443 and 40444). This addresses the world-map message "You cannot fast travel from this location", independently of the F2 teammate teleport.
- Survival, interior, combat and over-encumbrance origin restrictions no longer reject map travel while connected. The existing map destination selection, confirmation and native travel processing remain in place.
- The override requires a living local player with loaded 3D and an attached cell, and is inactive during loading/main menus. Offline play uses the original checks.
- F2 teammate teleport from v0.8.5 is retained. User confirmed this path works; normal map travel still needs two-PC gameplay verification for this hotfix.

## Direct player teleport hotfix

- F2 Teleport to Player uses the native COC relocation processor with the teammate's server-reported coordinates and no fast-travel marker. It bypasses map travel restrictions, including Survival, interiors, combat and over-encumbrance.
- Relocation runs on a game update after closing menus and waits for an existing load to finish. Exterior targets resolve the actual grid cell from their worldspace and coordinates.
- Arrival is checked against the destination world/interior and position. Missing destinations and load timeouts produce explicit messages; disconnect cancels pending relocation. The teleport console command uses the same path.
- Client and server reject unavailable or invalid destination coordinates.
- Two-PC gameplay testing is required for interiors, cross-worldspace travel and Survival. Automated tests cover input/arrival validation; they do not run the game.

## Survival, quests and personal companions

- Corrects the vanilla global Papyrus callback ABI (VM, stack, static base, arguments). The HUD re-enables Survival map fast-travel controls. The direct co-op teleport above is independent of map fast-travel restrictions.
- Third-person projectile launch angles converge on the engine crosshair ray hit, including local spell projectiles. Corrected angles are transmitted to peers. Projectile gravity, range and collision remain native; empty-sky launches retain native aim.
- Miscellaneous quest sync defaults on. Visible objective-only quests can sync before their first numbered stage. New side/misc quest progress made by a guest is proposed to the leader and published through the same authoritative journal; old guest quests are baselined rather than bulk-imported. Main-quest authority remains with the leader. If retaining an old STServer.ini, set bEnableMiscQuestSync=true.
- Holds the completed local rest clock briefly while the server acknowledges a sleep/wait skip so delayed sleep scripts do not observe the pre-sleep hour. Normal outdoor sleep limits and disease modifiers remain.
- Each player's vanilla follower receives an independent persistent actor reference after recruitment in a party. Two players can therefore have their own Lydia. The companion uses the existing temporary-actor replication path, with its own AI owner; saved personal companions are preserved when reconnecting. Third-party follower frameworks are not adapted. Original quest reference identities are retained; quest-bound inventory is not copied to a personal companion.
- Official Hearthfire and Goldenhills steward alias assignments sync alongside housing flags, including named actors and personal-companion replicas. Becoming a steward clears a matching local follower alias/count and refreshes AI packages. Role IDs are committed atomically so concurrent edits cannot mix identities.

Combat dependency fixes for the installed Skyrim 1.7.104 runtime: the current local SKSE log rejects the old Valhalla Combat and Ultimate Combat DLLs. Their ESPs alone do not run native combat or stamina features. Install matching runtime updates on both PCs; this package does not redistribute or replace those third-party DLLs. Valhalla 1.7.104 patch source: https://github.com/DeadOnKeyboard1/ValhallaCombat-v1.3.5-AE-1.7.104.0-Patch . Ultimate Combat NG DLL update: https://www.nexusmods.com/skyrimspecialedition/mods/163377?tab=files .

Two-PC gameplay validation is still required, especially follower recruitment, reconnect/save/load, steward dismissal and indoor sleep. Do not mix client/server versions: the journal proposal action and housing binding indices changed. Preserve config/CoopChest.dat.

## Party crash hotfix

- Removes dynamic native map-marker creation, remote-cell loading and handcrafted marker extra-data from the party HUD update. This is a suspected party-formation crash path, pending two-PC verification.
- Guards a missing camera node before accessing camera children and corrects screen projection to write depth into z rather than overwriting y.
- Retains compass markers, overhead names, current-location teleport and the other v0.8.2 features.

## Co-op fixes and HUD

- Shares item/gold grants issued by Papyrus quest and dialogue fragments (QF_/TIF_) with the current party. Tracks source fragment, callsite, item and occurrence to avoid doubling a reward when the same script executes locally and remotely. This does not invent rewards from a completed journal flag or copy unrelated loot. Custom reward systems outside these script paths need separate adapters.
- Party locations update every second, including distant players. Adds a compass overlay and projected overhead names for loaded party actors. Native world-map markers are temporarily disabled in v0.8.8 while investigating the crash when a party forms; current-location teleport buttons remain available.
- Adds direct "Teleport to <name>" buttons in F2 using the player's current server location.
- Adds a separate hunger bar driven by the installed official Survival Mode hunger globals, with percentage and stage label. It is hidden when Survival Mode is inactive.
- Restores the fast-travel control disabled by Survival Mode's watcher while retaining hunger, cold and exhaustion updates. Other normal engine travel checks remain in effect.
- Crop harvest requests validate world and distance instead of rejecting a nearby exterior plot across a cell boundary. Mature flora activation is unblocked, native harvest uses default processing, and rejected/unready requests provide feedback.
- Shared chest commits send one revisioned item delta instead of the complete inventory. Open chests reconcile only changed item variants immediately. Snapshot recovery handles missing revisions; the offline view cache is coalesced while server storage is still committed before acknowledgement.
- Replicates SJG Dual Wield Parrying's blockStart/blockStop graph inputs when its plugin is loaded. Reconciles local block intent after the legacy script updates its conditional state, with bounded retries if the animation is cancelled. Both PCs need matching parry/animation mods.
- F2 wait/sleep speed slider ranges from 1x to 10x, defaults to 3x, and retains the 168-hour/seven-day maximum.

Native build, protocol and policy tests are automated. Two-PC gameplay, mod compatibility, and physical chest clearance still need in-game testing.

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

Teleport crash repair: skip weapon-state polling for absent/unloaded actors during relocation; defer discovery when the destination grid cell is not available. Confirmed v0.8.6 null-actor crash signature from three October 21 dumps.

Expanded shared floor items: books (including Catalogue of Weapon Enchantments), notes, keys, torches and apparatus now use the same type policy as all other inventory items. Item IDs and mod IDs are resolved through the normal shared plugin mapping. Actor trade/pickup/drop hooks also carry generated alchemy definitions. Both PCs must have the source plugins and models installed.

