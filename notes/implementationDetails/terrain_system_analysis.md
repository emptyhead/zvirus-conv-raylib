# Terrain System Analysis: Raylib (C) vs. Blitz3D (Original)

This document provides a technical breakdown of the terrain system in the Zvirus project, comparing the current C/Raylib conversion with the original Blitz3D implementation.

## 1. Logic and Data Flow

### Initialization
- **Data Loading ([TerrainLoad](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#168-311) / `TERRAINload`)**:
  - Both versions load a 256x256 BMP (`LevelData/`) where color channels store metadata:
    - **Green**: Land height (scaled by 0.05).
    - **Red**: Object index (indices into `gGroundObjects`).
    - **Blue**: Land index (biomes: sea, beach, land, etc.).
  - The C version correctly ports the two-pass coloring logic, including random variations and calculation of "infected" purple variants.
- **Mesh Generation ([TerrainInitGrid](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#19-78) / `MESHgrid`)**:
  - A dynamic scrolling mesh is generated. It doesn't represent the whole level but a "view window" that follows the player.
  - The grid uses a "double-vertex" pattern at tile boundaries to allow for sharp coloring and culling.

### Update Loop
- **Grid Scrolling ([TerrainUpdateGrid](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#79-167) / `TERRAINupdate`)**:
  - The grid vertices are recalculated every frame relative to the player's position (`Ship *player`).
  - **Wrapping**: The `wrapi` function handles seamless world wrapping when the player crosses tile boundaries.
  - **Interpolation**: [TerrainGetHeight](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#313-351) uses bilinear interpolation to find the exact height at sub-tile coordinates.
  - **Wave Animation**: Water tiles (height < 0.25) have a sinusoidal vertical offset applied based on `gRipple` and a pre-calculated `waveFactor`.
- **Objects ([TerrainUpdateObjects](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#487-531) / [TerrainRenderObjects](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#395-486))**:
  - Objects are updated (sway, emission) and rendered separately in C, whereas Blitz3D handled this inside the main terrain loop.
  - C uses `rlgl` push/pop matrices to place and rotate each object.

---

## 2. Key Structures and Variables

### [Terrain](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#168-311) Struct (Core Data)
| Variable | Description |
| :--- | :--- |
| `landHeight` | Vertical position of the ground (from BMP Green). |
| `objectIndex` | ID of the object on this tile (from BMP Red). |
| `objectStatus` | 0=Normal, 1=Infected, 2=Destroyed. |
| `landInfected` | Boolean flag for infection status. |
| `r[2], g[2], b[2]` | Primary and Infected colors for the tile. |
| `waveFactor` | Pre-calculated offset for water ripple animation. |

### Global State
- `gGrid`: Configuration for mesh density and view distance.
- `gRipple`: Global counter for water waves (incremented by 15.0 per frame in BB).
- `gRotate`: Global counter for object animations (Radar/Mill/Crate).

---

## 3. Comparison and Differences

### Coordinate Systems
| Feature | Original (Blitz3D) | Conversion (Raylib/C) |
| :--- | :--- | :--- |
| **Handedness** | Left-Handed (Z+ is Forward) | Right-Handed (Z+ is Backward) |
| **Grid Vertices** | `VertexCoords ... -vz` | `vertices[... + 2] = vz` |
| **Rotations** | Pitch, then Yaw (Euler) | `-yaw` then `-pitch` using Raylib's `rlRotatef` |

### Timing and Frequency (30Hz vs 60Hz)
To maintain the original look and feel at 60Hz, several scaling factors are applied:
- **Object Sway**: Incremented by `0.25` in C (vs `0.5` in BB).
- **Object Cycle**: Wrapped at `40` in C (vs `20` in BB) to preserve the frequency of particle emissions.
- **Wave Speed**: If `gRipple` is updated at the same rate, waves will move twice as fast unless compensated in the update loop or global counter.

### Mini-map Logic
- **BB**: Uses a `MAPchange` buffer and `WritePixelFast` to incrementally update a texture every frame.
- **C**: [TerrainMapAdd](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#532-567) updates the `landInfected` status, but the incremental UI update logic for the minimap is pending full implementation in [hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c).

---

## 4. Missing Functionality

### 1. Dynamic Shadows
> [!WARNING]
> The original Blitz3D code used a second camera (`Shadow`) to render a top-down view of objects into a texture (`ShadowTexture`), which was then applied as a second texture layer on the terrain. This provided real-time shadows for all buildings and trees. **This is currently missing from the C conversion.**

### 2. Star and Rain Particles
- The original `TERRAINupdate` (lines 1658-1678) automatically spawns star or rain particles based on the player's altitude. 
- Rain frequency is tied to the total level infection rate (`AreaInfected`).
- This logic is not currently present in [TerrainUpdateObjects](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#487-531).

### 3. Starfield/Cloud Transitions
- Blitz3D changes the fog range and particle types when the player exceeds a certain height (Y > 25).

---

## 5. Limitations and Ideas for Expansion

### Limitations
- **Resolution**: Limited to 256x256 tiles by the `SIZE` constant.
- **CPU Bottleneck**: Recalculating all grid vertices and colors on the CPU every frame and uploading to GPU (`UpdateMeshBuffer`) is inefficient for modern hardware.
- **Floating Point Precision**: At the edges of the 256x256 world, some precision jitter might occur in interpolation.

### Expansion Ideas
- **Mesh Shaders**: Move the grid scrolling and wave animation logic to a Vertex Shader. Only the player position and `gRipple` would need to be passed as uniforms.
- **Voxel Terrain**: Convert the heightmap system into a ray-marched or voxel-based terrain for more vertical complexity.
- **Better Culling**: Implement frustum culling for ground objects instead of a simple distance-based range.
- **Dynamic Weather**: Expand the missing rain system into a full weather module with wind affecting tree sway speeds.
