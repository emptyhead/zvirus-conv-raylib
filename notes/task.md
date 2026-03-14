# Porting zvirus (Source.bb -> C/Raylib)

The objective is to incrementally port the BlitzBasic logic flow into C/Raylib while allowing tests at each phase. We have already noticed that foundational structures ([world.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/world.h), basic [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c), [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c), [player.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/player.c)) are partially implemented. 

## Iterative Phases

### Phase 1: Core Loop & Game State Flow
- **Goal:** Mirror the [Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb) loop: `MAINroutine` -> `GAMEstart` -> `WAVEstart` -> `GAMEupdate` -> `WAVEend` -> `GAMEend`.
- **Tasks:**
  - [/] Map `GAMEinit()`, `GAMEstart()`, `WAVEstart()` in [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c).
  - [/] Implement the `Timer` and `WaitTimer` logic to lock to 30 FPS (`FPS=30`).
  - [/] Setup the rendering sequence exactly like `RenderWorld()`.
- **Test:** Run the executable to ensure `STATE_MENU` correctly transitions to `STATE_WAVE` and an empty terrain renders at 30 FPS.

### Phase 2: Terrain & Camera
- **Goal:** Full translation of `TERRAINload`, `TERRAINgetheight`, and `TERRAINupdate`.
- **Tasks:**
  - [ ] Translate map loading pixel-by-pixel, assigning `TerrainType` properties exactly like the original code's `ReadPixel` loops.
  - [ ] Implement `CAMERAupdate()`, supporting `CAMERAoriginal`, `CAMERAchase`, and `CAMERArear`.
  - [ ] Translate `GridType` rendering (culling invisible terrain, wave animation on water).
- **Test:** Fly a phantom/free camera around the loaded `Map.bmp` and ensure heights match the original.

### Phase 3: Player Movement & Collisions
- **Goal:** Port `PLAYERupdate`, `PLAYERreset` and the fundamental collision sweeps.
- **Tasks:**
  - [ ] Implement `PLAYERupdate` logic (momentum, gravity, thrust, turning speed).
  - [ ] Implement raylib equivalent of BlitzBasic's `TFormNormal` and `RotateEntity` math (Quaternions).
  - [ ] Add `COLLISIONground` to handle crashing into mountains/water or picking up objects.
  - [ ] Add `COLLISIONplayer` for bouncing or damage between entities.
- **Test:** Control the player (ID 0) in the world, test crashing into terrain and bounding box logic.

### Phase 4: Enemy AI & Spawns
- **Goal:** Read AI spawn data accurately and execute AI navigation.
- **Tasks:**
  - [ ] Translate `PLAYERai()`, handling the various flying enemies (`AI=0` to `19`).
  - [ ] Port the `Restore WaveData` reading so enemies spawn per wave.
  - [ ] Hook AI up to the collision system so they can crash or bounce.
- **Test:** Start Wave 1, verify enemies spawn and navigate without crashing immediately (unless intended).

### Phase 5: Weapons, Particles, & Map/HUD
- **Goal:** Add shooting, missiles, particles, and the 2D overlays.
- **Tasks:**
  - [ ] Implement `PLAYERshootat()` and `PLAYERlaunch()`.
  - [ ] Implement `COLLISIONbullet()`.
  - [ ] Implement `PARTICLEnew()` and `PARTICLEupdate()`.
  - [ ] Translate `MAPupdate` to render the radar minimap.
  - [ ] Draw the HUD (`HUD\fuel`, `HUD\height`, `HUD\crosshair`).
- **Test:** Shoot enemies, watch explosions. Verify radar updates.

### Phase 6: Menus, Audio & Polish
- **Goal:** Sound effects, main menu flow, game over screens.
- **Tasks:**
  - [ ] Port `MENUupdate` and `MENUshow`.
  - [ ] Load `SOUNDmenu`, `SOUNDexplode`, etc., and replace `PlaySound` with Raylib's equivalents.
  - [ ] Finalize wave transitions.
- **Test:** Full playthrough of a wave with audio.
