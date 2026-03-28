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

---

Listed directory src
Viewed terrain.c:1-659
Viewed constants.h:1-346
Viewed main.c:1-789

This project uses a heightmap-based terrain system where the world is defined by a 256x256 image and rendered as a dynamic mesh centered on the player. Below is the exact breakdown of the formulae and values used for image sampling, mesh generation, and the Sea biome animation.

### 1. Image Sampling & Value Conversion
The terrain data is loaded in [TerrainLoad](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:169:0-315:1) (located in [src/terrain.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:0:0-0:0)) from a BMP file. Each pixel represents one "cell" on the 256x256 grid.

| Data Type | Image Channel | Conversion Formula | Resulting Range |
| :--- | :--- | :--- | :--- |
| **Land Height** | **Green** | `height = pixel.green * 0.05` | 0.0 to 12.75 |
| **Biome (Index)**| **Blue** | `index = (pixel.blue + 20) / 50` | 0 (Sea) to 4 (Infected) |
| **Object Index** | **Red** | `obj = (pixel.red + 10) / 20` | 0 to 11 (Bush, Trees, etc.) |

*   **Wave Factor Phase**: During loading, a static "wave factor" is pre-calculated for every cell based on its distance from the center of the map:
    *   **Formula**: `t->waveFactor = sqrt((x - 128)^2 + (z - 128)^2) * 45.0`
    *   This creates a radial phase shift used to make the sea waves ripple outwards from the center.

### 2. Mesh Construction & Coloring
The terrain is rendered as a single `Mesh` that follows the player. The vertices are updated every frame in [TerrainUpdateGrid](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:80:0-167:1).

*   **Grid Density**: The mesh uses a higher vertex density than the heightmap (4 vertices per heightmap cell) to allow for smooth scrolling and geometry.
*   **Vertex Height**: Usually matches `landHeight`, but is overridden by the wave formula if in the Sea biome.
*   **Biome Coloring**: Colors are assigned during the second pass of [TerrainLoad](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:169:0-315:1). Values below use `th`, which is the localized height scaled by 20.

| Biome | Index | Red Channel | Green Channel | Blue Channel |
| :--- | :--- | :--- | :--- | :--- |
| **Sea** | 0 | `60 + Rand(0, 39)` | `60 + Rand(0, 39)` | [(150 + th*10) + Rand(0, 39)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) |
| **Beach** | 1 | [(th*10 + 60)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) | [(th*10 + 60)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) | [(200 - th*10) + Rand(0, 39)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) |
| **Land** | 2 | `th + Rand(0, 59)` | [(th + 60) + Rand(0, 59)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) | `th + Rand(0, 59)` |

### 3. Sea Biome Animated Waves
The "Sea" biome logic (found in [TerrainUpdateGrid](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:80:0-167:1) lines 119-122) applies an oscillating vertical offset to any vertex where the base height is below the sea-level threshold.

#### The Formula
$$vy_{final} = vy_{base} + (0.25 - vy_{base}) \times (1.0 + \cos((gRipple + t.waveFactor) \times \frac{\pi}{180}))$$

#### Key Values:
*   **Sea Level Threshold**: `0.25`. This is the height at which the water is considered "flat" and the wave animation stops scaling.
*   **gRipple (Global Clock)**: Increments by **7.5** every frame (at 60 FPS). This value wraps from 0 to 360.
*   **Wave Factor**: The pre-calculated radial phase (distance from center * 45.0).
*   **Behavior**: 
    *   At the "deepest" point ($vy = 0$), the wave oscillates between **0.0** and **0.5** height.
    *   As the terrain gets shallower ($vy$ approaches $0.25$), the amplitude of the wave smoothly decreases to zero, creating a natural shoreline effect where the water meets the beach.
    
---

"Values below" refers to the specific RGB color formulas for each biome listed in the **Biome Coloring** table (in the [src/terrain.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:0:0-0:0) pass). 

Inside that pass (at line 253), the code calculates a variable `th` once per cell, which is the **interpolated terrain height multiplied by 20.0f**. 

```c
// src/terrain.c:253
float th = TerrainGetHeight((float)x + 0.5f, (float)z + 0.5f, 0) * 20.0f;
```

The formulas in the table specifically use this `th` value to modulate the brightness and hue based on altitude. 

| Biome | Channel | Where `th` is used |
| :--- | :--- | :--- |
| **Sea** | **Blue** | `150 + (th * 10) + Rand(0,39)` |
| **Beach** | **Red/Green** | [(th * 10 + 60)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) |
| **Beach** | **Blue** | [(200 - th * 10) + Rand(0,39)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) |
| **Land** | **All Channels**| [(th + offset) + Rand(0,59)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) |

### Why scale by 20.0f?
The heightmap values (green channel) are multiplied by `0.05f` during the first pass to create the "world scale" height (which is typically a small number like 0.0 to 12.0). 

When it comes to coloring, the code multiplies that world-scale height back by **20.0f** (effectively restoring the original pixel value from the image) to use as a brightness intensity. This ensures that even tiny changes in height result in a visible change in the color depth of the quad.