# Implementation Plan: zvirus (Source.bb -> C/Raylib)

The goal is to port a ~3000 line BlitzBasic game to C/Raylib incrementally. We already have structural roots ([world.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/world.h), [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h)) capturing the structs (`TerrainType`, `PlayerType`, etc.). This plan delineates the technical approach to preserving the logic flow faithfully.

## Proposed Changes

### 1. Game State and Loop Synchronization
Files: [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c), [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c), [game.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.h)
- Update the main loop to match the `MAINroutine` flow. 
- Ensure `FPS` locking uses Raylib's `SetTargetFPS(30)` or custom delta time accumulator so game logic ticks at 30Hz as the original `Timer` did.
- Initialize `GameContext` mirroring missing globals (e.g., `Grid`, `AlliesON`, `ActiveParticles`).

### 2. Math Translation (BlitzBasic 3D -> Raylib)
Files: [util.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/util.c), [util.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/util.h) (or inline math)
- BlitzBasic largely relies on `RotateEntity`, `MoveEntity`, `TFormNormal`, `TFormedX/Y/Z()`.
- We will need mapping functions like `EntityPitch`, `EntityYaw` converted to direction vectors and back. Raylib's `MatrixRotateXYZ` and `Vector3Transform` will be key here.

### 3. Terrain System
Files: [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c), [terrain.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.h)
- Implement `TERRAINload` fully. BlitzBasic uses `ReadPixel()`. Raylib provides `LoadImage` and `GetImageColor`. We will read RGB values out of `Map.bmp` to construct `Terrain[x][z]`.
- Implement `TERRAINupdate()`. In Raylib, generating dynamic meshes on every frame is slow; we will upload a static mesh and do wave animation using a vertex shader, or update `Mesh.vertices` mapping using `UpdateMeshBuffer()`. Given `Size=256`, updating it every frame via CPU might be acceptable if optimized, but a static mesh is better.

### 4. Entity Update Pass
Files: [player.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/player.c), [player.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/player.h)
- Recreate `PLAYERupdate()` identically. Loop over `100` max players. 
- Extract inputs if `ID == 0` (Translate `CONTROLmouse`, `CONTROLoriginal` modes to Raylib's `GetMouseX`, `GetMouseDelta`).
- Implement the AI modes matching the `Select AI` routines.

### 5. Collision Systems
Files: `collision.c`, `collision.h` [NEW]
- Create standalone collision loop files to declutter the `PLAYERupdate`.
- Translate `COLLISIONground()`, `COLLISIONplayer()`, `COLLISIONbullet()`. Raylib provides simple `CheckCollisionSpheres()`. 

## Verification Plan

We will verify incrementally:
1. **Automated / Manual Build Check:** `make run` must successfully render the window at each stage.
2. **Terrain Inspection:** Free-fly camera over the Raylib terrain, comparing visual layout (mountains vs. water) to [Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb).
3. **Control Feel Check:** The mouse momentum (`MouseSens`) is a distinctive part of [Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb). The port handles mouse delta and mapping to YAW/PITCH similarly. Testing this requires compiling and running via X11.
