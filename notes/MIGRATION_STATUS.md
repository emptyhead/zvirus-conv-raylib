# Migration Status: Blitz3D to C/RayLib

## Global State & Types
| Blitz3D Type | C Struct (world.h) | Status | Notes |
| :--- | :--- | :--- | :--- |
| `SoundGroupType` | `SoundGroup` | [DONE] | Fully mapped. |
| `MenuType` | `Menu` | [DONE] | Supports sub-menus. |
| `ScoreTagType` | `ScoreTag` | [DONE] | Struct exists, system missing. |
| `ParticleTemplateType` | `ParticleTemplate` | [DONE] | All 22 templates mapped. |
| `ParticleType` | `Particle` | [DONE] | Mapped. |
| `GroundObjectType` | `GroundObject` | [DONE] | Supports 3 model states. |
| `FlyingObjectType` | `FlyingObject` | [DONE] | All physics constants included. |
| `GridType` | `Grid` | [DONE] | Mapped. |
| `TerrainType` | `Terrain` | [DONE] | Includes `landHidden` for radar. |
| `HudType` | `Hud` | [DONE] | Mapped. |
| `PlayerType` | `Ship` | [DONE] | Includes AI intent flags. |

## System Mapping (Execution Order)
| Step | Source.bb Loop | main.c Loop | Status |
| :--- | :--- | :--- | :--- |
| 1 | `WaitTimer` | `accumulator` (Fixed Step) | [DONE] |
| 2 | `PLAYERupdate` | `ShipUpdateAll` | [DONE] |
| 3 | `TERRAINupdate` | `TerrainUpdateGrid` / `TerrainRenderObjects` | [DONE] |
| 4 | `CAMERAupdate` | `CameraGameUpdate` | [DONE] |
| 5 | `PARTICLEupdate` | `ParticleUpdateAll` | [DONE] |
| 6 | `SCOREtagupdate` | `ScoreTagUpdateAll` / `ScoreTagDrawAll` | [DONE] |
| 7 | `MAPupdate` | `HudUpdateMap` (every 60 frames) | [PARTIAL] |
| 8 | `RenderWorld` | `BeginDrawing` block | [DONE] |
| 9 | `GAMEupdate` | `GameUpdate` | [PARTIAL] |

## Feature Parity Checklist

### 1. Game Environment
- **[DONE]** `TERRAINgetheight` -> `TerrainGetHeight`: Precise parity including object height checks.
- **[PARTIAL]** `TERRAINload` -> `TerrainLoad`: Biome coloring ported, but `WaveData` loading needs verification against `Source.bb:310`.
- **[DONE]** `TERRAINupdate`: Basic mesh scrolling, object rendering, and water waves (`Source.bb:1708`) ported. Environment particles (Rain/Stars) moved to `GameUpdate` in `game.c`.
- **[DONE]** `MESHgrid` -> `TerrainInitGrid`: Correctly generates LOD/culling grid.
- **[DONE]** `GroundObjects` meshes: Fully implemented multi-state mesh system (Normal, Infected, Destroyed). Fixed case-sensitivity, composite parts (Radar/Mill), and automated height calculations.
- **[DONE]** FlyingObject `MESHload` -> `LoadMy3D`: Custom C implementation to parse Blitz3D `.my3d` binary meshes into Raylib `Model`s, with Z-axis and winding flips for right-hand coordinates. Uses 60Hz global `gRotate` ticks for animations.
- **[DONE]** `PLAYERchase` -> `ShipChase`: Handled with coordinate flip for RayLib.
- **[DONE]** `PLAYERfindtarget` -> `ShipFindTarget`: Mapped.
- **[DONE]** `PLAYERai` -> `ShipAI`: All engine types (0-3) mapped.
- **[PARTIAL]** `PLAYERattract` -> `ShipUpdateAll` (intent logic): Needs full implementation of the beam visual and pull force (`Source.bb:1039`).
- **[PARTIAL]** `PLAYERshootat`: Basic shooting exists, but turret `PointEntity` logic (`Source.bb:1090`) needs integration for ID 9 (Repulsor).
- **[DONE]** `PLAYERlaunch` -> `ShipLaunch`: Child/Missile spawning ported.
- **[DONE]** `Monster Spawn`: Monster (ID 17) only spawns in water regions at random positions.

### 3. Collision & Physics
- **[DONE]** `COLLISIONplayer` -> `CollisionShip`: Elastic bounce and faction damage ported.
- **[DONE]** `COLLISIONground` -> `TerrainCollisionGround`: Object destruction and Radar disabling mapped.
- **[DONE]** `COLLISIONbullet` -> `CollisionBullet`: Particle-Ship collision mapped.
- **[PARTIAL]** `GAMEupdate: targeting`: `LinePick` logic for crosshair targeting (`Source.bb:1442`) is missing.

### 4. Particles & Polish
- **[DONE]** `PARTICLEupdate`: Physics/Physics ported. Billboarding implemented via `DrawBillboard` and generated sparkle texture.
- **[DONE]** `SCOREtagupdate`: Floating score system (`Source.bb:1245`).
- **[MISSING]** `FADE`: Screen fade transitions (`Source.bb:2021`).
- **[PARTIAL]** `SOUNDtimer`: Ambient thunder/wind modulation (`Source.bb:912`).

## AI Gap Analysis (Types 0-16)

The following table compares the original Blitz3D (30Hz) values with our current C/RayLib (60Hz) implementation. 

### Timing & Coordinate Conversion Rules:
*   **Thrust**: Target C value should be `Blitz / 2` (since integration step `0.5f` is applied, but updated twice per 30Hz interval).
*   **Momentum**: Target C value should be `sqrt(Blitz)` (to maintain decay curve over 2x frames).
*   **Turn Speed**: Target C value should be `Blitz / 2` (since it's applied every frame with no integration halving).
*   **Fire/Missile Rate**: Target C value should be `Blitz * 2` (frames between shots).

| ID | Name | Blitz T/M/TS/FR | Current C T/M/TS/FR | Target C (60Hz) | Status | Gap Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 0 | Hoverplane | 0.02 / 0.9925 / 5 / 3 | 0.01 / 0.9975 / 2.5 / 6 | 0.01 / 0.9962 / 2.5 / 6 | [OK] | Mom slightly high (0.9975 vs 0.9962). |
| 1 | Seeder | 0.001 / 0.99 / 5 / 3 | 0.00375 / 0.995 / 2.5 / 12 | 0.0005 / 0.995 / 2.5 / 6 | [FAIL] | **Thrust 7.5x too high.** Fire rate too slow. |
| 2 | Bomber | 0.002 / 0.99 / 5 / 20 | 0.0075 / 0.995 / 2.5 / 72 | 0.001 / 0.995 / 2.5 / 40 | [FAIL] | **Thrust 7.5x too high.** Fire rate too slow. |
| 3 | Pest | 0.04 / 0.9 / 5 / 2 | 0.005 / 0.97 / 2.5 / 8 | 0.02 / 0.9487 / 2.5 / 4 | [FAIL] | **Thrust 4x too low.** Mom too high. |
| 4 | Drone | 0.015 / 0.98 / 2 / 15 | 0.0075 / 0.99 / 1.0 / 30 | 0.0075 / 0.9899 / 1.0 / 30 | [OK] | Matches target. |
| 5 | Mutant | 0.015 / 0.98 / 3 / 12 | 0.0075 / 0.99 / 1.5 / 48 | 0.0075 / 0.9899 / 1.5 / 24 | [PARTIAL] | Fire rate too slow. |
| 6 | Fighter | 0.015 / 0.98 / 4 / 9 | 0.0075 / 0.99 / 2.0 / 36 | 0.0075 / 0.9899 / 2.0 / 18 | [PARTIAL] | Fire rate too slow. |
| 7 | Destroyer | 0.0175 / 0.98 / 5 / 9 | 0.00875 / 0.99 / 2.5 / 36 | 0.00875 / 0.9899 / 2.5 / 18 | [PARTIAL] | Fire rate too slow. |
| 8 | Attractor | 0.0015 / 0.99 / 5 / 2 | 0.00075 / 0.99 / 2.5 / 8 | 0.00075 / 0.995 / 2.5 / 4 | [PARTIAL] | Mom low, Fire rate slow. |
| 9 | Repulsor | 0.0025 / 0.99 / 5 / 9 | 0.00125 / 0.99 / 2.5 / 36 | 0.00125 / 0.995 / 2.5 / 18 | [PARTIAL] | Mom low, Fire rate slow. |
| 10 | Mystery | 0.0015 / 0.99 / 5 / 2 | 0.00075 / 0.99 / 2.5 / 8 | 0.00075 / 0.995 / 2.5 / 4 | [PARTIAL] | Fire rate slow. |
| 11-14 | Generators | 1e-6 / 0.0 / 5 / 3 | 7.5e-8 / 0.0 / 2.5 / 12 | 5e-7 / 0.0 / 2.5 / 6 | [FAIL] | Thrust too low. Fire rate slow. |
| 15 | Cruiser | 0.004 / 0.99 / 20 / 3 | 0.009 / 0.99 / 10.0 / 12 | 0.002 / 0.995 / 10.0 / 6 | [FAIL] | **Thrust 4.5x too high.** Fire rate slow. |
| 16 | Elite | 0.02 / 0.9925 / 5 / 6 | 0.01 / 0.9962 / 2.5 / 24 | 0.01 / 0.9962 / 2.5 / 12 | [PARTIAL] | Fire rate slow. |
| 17 | Monster | 0.00001 / 0.0 / 3 / 2 | 0.000005 / 0.0 / 1.5 / 4 | 0.000005 / 0.0 / 1.5 / 4 | [DONE] | Spawns in water only. |

### Summary of Findings
1.  **Global Over-speeding**: Several AI types (Seeder, Bomber, Cruiser) have thrust values significantly higher than the mathematically derived 60Hz equivalents.
2.  **Firing Rates**: Almost all AI firing rates are set to 4x the original delay instead of 2x (e.g. `G_FIRE_RATE * 12` for Bomber).
3.  **Momentum**: Most momentum values are using `0.99` instead of `sqrt(orig)`. For `0.98` orig, target should be `0.9899`.
4.  **Rotation**: Player and Drone rotation appear correct, but non-player rotation should be verified for the 2x frequency.

## Logic & Parity Notes

### Coordinate System
- **Z-Axis**: Flipped as expected. `Source.bb:1152` vs `ship.c:685`.
- **Rotation**: Blitz Euler angles (Pitch, Yaw, Roll) are CW. RayLib `rlRotatef` defaults to CCW. Negation in `main.c:282` correctly handles this.

### 60Hz Tuning
- **Gravity/Thrust**: Constants in `constants.h` are roughly 50% of Blitz values to account for `dt=0.5`.
- **Momentum**: Higher momentum (`0.9975`) used to approximate 30Hz friction decay over twice as many frames.
- **WaitTimer**: Replaced by `accumulator` logic in `main.c` for smooth 60fps simulation with fixed physics steps.

## Next Steps Recommended
1. **Targeting UI**: Implement the crosshair `OnTarget` scaling and scaling logic from `GAMEupdate`.
2. **Sound Modulation**: Port ambient thunder/wind modulation logic (`Source.bb:912`).
