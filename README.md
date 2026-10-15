# Survival Sync alpha 0.3 — test build

- F2: optional auto resync interval (1–300 seconds, default 10).
- F2: bind or clear a single resync hotkey. F2 and modifier-only bindings are reserved.
- Manual and automatic refresh cooldown reduced to one second on client and server.
- Weather uses the earliest connected loaded player as a single server-wide source,
  independent of party membership/range; refresh/recovery every five seconds.
  It synchronizes the selected weather, not the exact visual blend progress.
- Native local sleep/wait time simulation is allowed; elapsed days already processed
  locally are deducted from the server skip, including overlap during another sleep.
  Awake players still accrue elapsed Survival fatigue; they do not receive sleep benefits.
- Experimental TK Dodge graph-input adapter forwards TKDodge-prefixed player events
  and standard animation variables plus roll/step selection; old dodges are not replayed on resync.
  Identical TK Dodge SE assets/behavior generation required on both PCs.
  This is not general compatibility with all animation mods or custom graph variables.
- Remote casts resolve the spell in the packet rather than stale equipped-hand state.
- Item movement remains 15 Hz with a three-second settling ownership delay.

## Open issues

Volcano's scripted eruption/secondary-source replication is NOT resolved. Added logging
for spell routing and projectiles launched by non-actor sources to diagnose it.
The matching Apocalypse 10.3 spell/plugin/script files are needed to finish its adapter.

Inn rental flag isolation is implemented for JobInnkeeperFaction members: Variable09
stays local, allowing each player to rent locally instead of inheriting another player's
already-rented flag. Existing save flags may require the old rental to expire. This does
not add extra rooms or permit simultaneous use of the same occupied bed; shared-bed
ownership remains unimplemented. Modified rental scripts need separate validation.

Sleep, weather and TK Dodge behavior require two-player in-game validation. Automated
policy and packet tests do not establish game-engine behavior. Use the same custom
build on both PCs and server; weather packet format changed.

---

# Skyrim Together: Survival + World Items alpha 0.2

Unofficial Windows build with the previous Survival time changes plus server-tracked
player drops and movable loose world items. Download the successful Actions artifact,
extract the outer artifact ZIP, and install the inner Windows ZIP using Vortex.
Disable the old build, deploy, and use the same custom client AND server on both PCs.

## World items

- Drops get one server identity, with inventory deduction checked before publication.
- Pickup is approved by the server; simultaneous or repeated pickups award the stack once.
- Grabbing transfers exclusive movement control. Position/rotation updates are sent at 10 Hz.
- Copies retain dynamic, activatable physics so either player can grab them.
  Non-owners correct toward the shared pose; the server only accepts the owner's movement.
  After release, the owner continues updates until the item settles.
- Snapshots restore the current state for nearby players and hide already-consumed items.
- Creation acknowledgement prevents picking up an unpublished local drop.

Target types: weapons, armor, ammo, ingredients, potions, miscellaneous items, scrolls,
and soul gems. Quest-aliased items and books retain their existing handling. This is
not a general synchronization system for furniture, doors, actors, or scripted objects.
NPCs are blocked from picking up tracked items in this alpha to avoid bypassing claims.
Custom names/ownership metadata are limited by Reborn's existing inventory format.

Server records are memory-only and have a 4096-record limit, including consumed
records. This is intended for an active test session, not persistent world hosting.
There is no durable save/restart ledger. Disconnect cleanup removes temporary replicas;
network failure during an unacknowledged drop can still need recovery. Full cheat-proof
inventory validation is not provided by Reborn's client-driven world architecture.

Movement between exterior cells in the same world is supported; interior/world
teleports are not a physics transfer path. Grab detection uses the AE PlayerCharacter
layout of the pinned Reborn base (0x8D0, documented by CommonLibSSE-NG).

## Validation

Native Windows build, request packet serialization, and assertion-enabled ownership /
race / replay policy tests must pass before the workflow uploads a package. Those tests
do not establish in-game physics behavior. Test new separate saves with ordinary iron
weapons or potions before valuable, enchanted, or modded gear. Test a 12-item stack,
simultaneous pickup, alternating grabs, moving in a dungeon and outdoors, and late join.

Survival hunger/fatigue/time behavior was reported working by the user in alpha 0.1;
the Survival time implementation is retained. Cold and other gameplay remain subject
to in-game validation. See SOURCE-NOTES.md for the previous patch's provenance.

Source base: tiltedphoques/TiltedEvolution commit
9ce0338522277e0ff0a12e8d2aa09ce67ad41a59. The complete modification is world-items-v0.2.patch.
GPL-3.0-or-later under the included upstream license.
