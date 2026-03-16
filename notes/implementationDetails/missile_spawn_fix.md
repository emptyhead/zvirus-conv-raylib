# implementation Detail: Missile Spawning Fix

## Problem
At the start of Wave 1 (and subsequent waves), missiles (AI type 20) were appearing and flying around immediately. In the original Blitz3D source (`Source.bb`), missiles and other "child" entities (like spores or elite guards) are initialized as inactive/dead objects stored within a parent entity. They should only become active when explicitly launched.

The C port was incorrectly initializing all entities spawned in `WaveStart` as active (`dead = 0`), including children.

## Solution
I modified the `ShipReset` function to accept an initial `dead` status, allowing the spawning logic to distinguish between active primary units and inactive stored children.

## Files & Changes

### 1. [src/ship.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.h)
- **Line 10**: Updated the `ShipReset` declaration to include the `int dead` parameter.
```c
void ShipReset(int id, int aiType, int dead);
```

### 2. [src/ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c)
- **Line 65**: Updated `ShipInitAll` to initialize the player with `dead = 0`.
- **Line 71**: Updated the `ShipReset` definition to use the provided `dead` parameter instead of hardcoding `0`.

### 3. [src/game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c)
- **Lines 127 & 132**: Updated test level spawning to use `dead = 0`.
- **Line 151**: Updated normal spawning loop to use `dead = 0` for primary wave enemies.
- **Line 174**: Updated child/missile spawning loop to use `dead = -1`, ensuring they start inactive as intended.

### 4. [src/main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c)
- **Line 672**: Updated player reset call (after death) to pass `0` for active respawn.

## Result
Missiles now start in a "truly dead" state (`-1`) and are only activated via `ShipLaunch`, matching the original game logic and cleaning up the level start.
