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
