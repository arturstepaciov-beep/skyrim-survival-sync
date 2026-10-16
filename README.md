# Survival Sync v0.6 draft — separate shared-chest network

**Source preparation only: no v0.6 Windows package has been built or tested in Skyrim.**

Adds a dedicated empty chest base and native access points at Goldenhills plus the
nine hold capitals. All access points share server-owned storage in
`config/CoopChest.dat`. Keep this file across server upgrades/restarts.
The server validates exact-variant transfers and persists storage before sending
the commit. Clients retain the last synchronized contents for a read-only offline
viewer; offline inventories never seed or merge into the server.

Goldenhills is outside the farmhouse's lower approach. Whiterun, Solitude,
Windhelm, Riften and Markarth are near their stables. Dawnstar, Falkreath, Morthal
and Winterhold are beside their inns. Final ground/path clearance is unverified.

The future native package must include and enable `SurvivalSyncCoop.esp` on both
PCs, with the same client/server version. It contains one new script-free empty
container and overrides no existing game records. Bethesda/CC masters and assets
are not bundled. Construction quests and ordinary home storage remain separate.

Implemented draft code and tests are in the source patch under
`Tools/CoopChests/README.md`. Local storage/ownership policy tests pass. New
packet tests, full Windows compilation, UI build and two-player validation are
pending the next explicit build request. Server storage, player saves and network
acknowledgements are not one crash-safe transaction; do not claim full crash or
save-rollback duplication protection.

## Retained v0.5 work


Unofficial matching Windows client/server build. Install the same timestamped ZIP
on both PCs AND restart the server. Existing Survival, weather, item movement,
TK Dodge and Apocalypse Volcano adapters are retained.

## Optional 144 FPS physics test package

The dedicated package-144fps workflow repackages the verified v0.5 binaries from
run 38902846919 with Profiles/144FPS. Native binaries are not recompiled or
modified. ProfileBuildInfo.json records both the native build and profile package
identity and binary checksums; the in-game native version remains unchanged.

Install SSE Display Tweaks and matching requirements separately on both PCs.
The included setup merges a 144 FPS gameplay / dynamic Havok profile into its
custom INI with a backup, preserving other settings. Loading and menus stay capped
at 60 FPS. No third-party physics DLL is bundled and no in-game compatibility or
144 FPS physics guarantee is implied. See Profiles/144FPS/README.txt for setup,
rollback and mixed 60/144 FPS tests.

## Autosync HUD and running/jumping adjustments

- A small top-left countdown shows seconds until the next automatic sync request.
  It uses the native scheduler, follows manual resets and interval edits, and
  hides when disconnected, autosync is off or game menus are open.
- Remote animation graph switches follow the displayed snapshot time instead
  of anticipating the next position. Due action batches are played in the same
  frame (bounded to 32), reducing extra takeoff/landing/start/stop delays.
- Humanoid graphs additionally synchronize sprint/moving flags, sprint state
  and jump blend. Native jump/fall/land and movement start/stop actions survive
  the noisy-notification filter.
- These are targeted playback changes, not universal animation-mod support.
  Both players need matching behavior/animation assets. Test walk/run/sprint,
  repeated jumps, landing while moving and TK Dodge with two players.
- Native countdown/timeline regression tests run before packaging. They cannot
  confirm Skyrim graph behavior; in-game testing remains required.

## Hearthfire houses follow the host

- Covers placed architecture, enable-parent markers, furniture, doors, lights and
  storage references owned by HearthFires.esm (Lakeview, Windstad, Heljarchen).
- The first connected player whose character is assigned is the session host.
  Connect the house owner first. Party leadership does not choose the house host.
- Guests receive enabled/disabled states from that host. Newly built or removed
  furnishings are published on the next one-second scan; ten-second snapshots
  recover state for late joiners.
- The index is read locally from the installed HearthFires.esm; no game assets
  or modified Bethesda plugins are bundled. This is not a universal mod-home adapter.
- The host must load each house and its storage once per server session so the
  native save inventory is available. A guest cannot seed an unseen chest from
  their own save. Storage becomes available after the host's snapshot arrives.
- This synchronizes placed structure visibility and storage. It does not copy
  ownership quests, adoption, stewards, NPC schedules, navmesh or building menus.
  Construction is performed by the host; guests see the resulting placed objects.

## Shared cooperative home chests

- Ordinary Hearthfire containers without quest aliases become shared cooperative storage; there is no
  separate chest plugin to install or console-spawned replacement chest.
- Open waits for the server's current contents. Deposits and withdrawals are
  intercepted before local mutation; the server commits both inventory changes.
- Networkable item variants and source quantities are checked, transfers have monotonic
  request tokens, and simultaneous withdrawals cannot spend the same last item.
- Large snapshots are split into bounded chunks and applied only when complete.
  Limits: 4096 distinct entries per inventory, 64 custom effects per entry and
  32767 items per individual transfer. Quest items cannot be transferred here.
  Withdraw to your inventory before dropping items into the world or forwarding
  them to NPCs. Quest-bound construction/resource containers retain their native
  scripts and are excluded from this adapter.
  Unequip worn items before depositing them. Temporary base forms (including
  custom potions) and temporary enchantment/poison forms are not transferred.
  Existing local dynamic and quest items are retained in place during reconciliation;
  they are not part of the shared inventory.
- The session state lives in server memory; the host receives committed contents
  in the native save when its reference is available. Visit affected houses and
  save on the host before stopping the server. On a clean
  guest disconnect the guest's original house/storage are restored; items acquired
  through completed transfers stay with the player. A crash cannot run that cleanup.

## Validation

This is an experimental in-game test build. Automated tests check indexing,
quantity conservation, replay rejection, packet chunk assembly and inventory
variant identity. They do not prove Skyrim engine behavior. Two-player validation
is still required, including different house progress, late joins, simultaneous
last-item withdrawal, enchanted items, reconnects and host save/reload.

Keep the original Apocalypse 10.3.0 installed on both PCs for Volcano. No Apocalypse
plugin, scripts, meshes or textures are included in this package.

## Volcano adapter

- The native Spell.RemoteCast path records the actual local blame actor for
  Volcano's lava-bomb spell. Projectiles emitted from temporary references can
  then use the existing actor-based projectile synchronization.
- The original caster alone runs the eruption controller. Nearby peers receive
  replicas of the script-free fountain and light forms, not a second controller.
- Visual replicas have independent creator/reference identities and never enter
  inventory or world-item pickup processing.
- A 250 ms source heartbeat restores active visuals for nearby late joiners.
  Removal is propagated when a source disappears; replicas also expire after
  two seconds without a heartbeat and are removed on disconnect.
- Form IDs are resolved through the installed Apocalypse plugin and the normal
  per-client mod mapping. The adapter is specific to the inspected 10.3.0 records.
- Damage remains on the existing projectile/magic synchronization path, including
  the server's PvP setting. The adapter does not inject a second damage routine.

This is an in-game test build, not a confirmed Volcano fix. Native RemoteCast
attribution, visual spawning, damage and cleanup still need a two-player check.
Test in both casting directions, indoors/outdoors, two simultaneous casts and
moving away/reconnecting. The initial one-shot eruption explosion is not separately
replicated by this adapter; the target is the persistent fountain/light and bombs.

## Retained features

- Shared Survival time, local native sleep/wait and elapsed-day deduplication.
  Awake players do not receive another player's sleep benefits.
- Dropped-item and loose-object movement sync at 15 Hz, with a three-second
  stillness window before movement ownership is released.
- F2 manual resync, optional automatic interval of 1–300 seconds, and a bindable
  single-key resync shortcut. F2 and modifier-only binds are reserved.
- A server-wide weather source selected from the earliest connected loaded player,
  with a five-second recovery refresh. Exact weather blending is not replicated.
- Experimental TK Dodge-prefixed graph inputs and roll/step selection. Matching
  TK Dodge SE assets and generated behaviors are required on both PCs.
- Innkeeper Variable09 rental flags remain local. Existing stale save flags are
  not automatically repaired. This does not add extra rooms or allow simultaneous
  occupancy of one bed, and shared-bed ownership remains unimplemented.

## Installation and validation

Use this exact custom build on both PCs AND the server; restart all three.
Preserve the existing STServer.ini and its enabled Survival/item-drop settings.
Extract the outer Actions artifact ZIP to obtain the installable Windows ZIP.
Each build carries its Vilnius start timestamp in the package/artifact name and the client/server build identifier. BuildInfo.json inside the ZIP records local/UTC time and source commit.

The workflow must compile the native client/server, run Survival/world-item/weather
packet tests and ownership-policy tests, and build the UI before uploading a package.
Automated tests do not prove Skyrim engine behavior.

## Source and license

Pinned upstream: tiltedphoques/TiltedEvolution
9ce0338522277e0ff0a12e8d2aa09ce67ad41a59.
The complete source modification remains world-items-v0.2.patch (the historical
filename is retained for the build script). GPL-3.0-or-later under the upstream
license. This build is unofficial and not endorsed by Skyrim Together's maintainers.
The inspected third-party Apocalypse archive is not redistributed.
