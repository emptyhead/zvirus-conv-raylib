# Terrain System: Implementation Logic & Data Flow

This document details the internal logic and data structures used for the terrain system in ZVirus, intended for developers wanting to replicate or extend the core system from scratch.

---

## 1. Source Data (RGB Metadata)

The world is driven by an image file (historically $256 \times 256$ BMP). Every pixel acts as a data packet for a single tile. Modern implementations can support any image format (PNG, JPG, EXR) and arbitrary dimensions.

### **Red Channel: Ground Objects**
Objects are placed at the integer center of the tile.
*   **Decoding:** `objectIndex = (PixelRed + 10) / 20`
*   **Indices:** `0=None, 1=Bush, 2-4=Trees, 5-7=Houses, 8=Radar, 9=Windmill, 10=Rocket, 11=Crate`.
*   **Yaw:** Randomized on load ($0-359^\circ$).

### **Green Channel: Elevation**
Direct linear mapping to the Y-axis.
*   **Decoding:** `landHeight = PixelGreen * 0.05`
*   **Range:** $0.0$ to $12.75$ units.

### **Blue Channel: Biome / Land Index**
Determines visual style and gameplay behavior.
*   **Decoding:** `landIndex = (PixelBlue + 20) / 50`
*   **Types:**
    *   `0`: Sea (Animated waves, default infected).
    *   `1-2`: Beach / Land (Standard infectable terrain).
    *   `3`: Landing Pad (Concrete, triggers refueling).
    *   `4`: Alien Hub (Pre-infected, radar jamming).

---

## 2. The Scrolling Skirting Grid

The renderer uses a **dynamic mesh** rather than a static map. This mesh stays centered on the player and samples the heightmap beneath it.

### **Displacement Calculation**
To achieve sub-pixel smoothness, the mesh "slides" using the player's fractional position:

1.  **Anchor Calculation:**
    `ax = floor(player.x)`, `az = floor(player.z)`
2.  **Fractional Offset:**
    `fx = player.x - ax`, `fz = player.z - az`
3.  **Vertex Positioning:**
    For a vertex at local grid offset `(dx, dz)`, the position in the mesh buffer becomes:
    *   `Vertex.x = dx - fx`
    *   `Vertex.z = dz - fz`
    *   `Vertex.y = gTerrain[wrap(ax + dx)][wrap(az + dz)].landHeight`

This ensures that as the player moves `0.1` units, the ground moves `-0.1` units relative to the camera, maintaining a perfect visual lock.

---

## 3. Flight Physics & Surface Interaction

Physics ignores the discrete nature of the mesh and treats the ground as a continuous mathematical surface.

### **Bilinear Surface Interpolation**
When an object is at non-integer coordinates `(x, z)`, the ground height is found by lerping between the 4 nearest neighbors:
1.  Sample heights `h00, h10, h01, h11` from the integer tiles surrounding `(x, z)`.
2.  `lerpX_top = lerp(h00, h10, frac(x))`
3.  `lerpX_bottom = lerp(h01, h11, frac(x))`
4.  `finalHeight = lerp(lerpX_top, lerpX_bottom, frac(z))`

This provides smooth vertical transitions for flight and prevents "jitter" when objects are moving at high speeds.

---

## 4. Proposed Modernization (Shader Path)

The current CPU-based vertex update is a performance bottleneck. A modern implementation should leverage GPU hardware.

### **GPU Architecture**
*   **Unified Map Texture:** Store `Height`, `Object`, and `Biome` in a single RGBA texture.
*   **Static Mesh:** Use a single, high-density vertex buffer ($256 \times 256$ vertices) that never changes.
*   **Vertex Shader:**
    ```glsl
    // Uniforms: vec2 playerPos, float mapSize
    vec2 uv = (position.xz + playerPos) / mapSize;
    float h = texture(heightMap, uv).g * 0.05;
    gl_Position = mvp * vec4(position.x, h, position.z, 1.0);
    ```

### **Pros and Cons**
| Approach | Pros | Cons |
| :--- | :--- | :--- |
| **CPU (Current)** | Easy to implement infection logic; simple debugging. | Heavy CPU load; limited to low vertex counts; flat shading only. |
| **GPU (Shader)** | Near-zero CPU cost; infinite vertex density; smooth per-pixel lighting. | Complex to sync "Infection" textures; requires harder math for collision. |

### **Recommendation**
Handle the **Gameplay State (Infection/Objects)** in a CPU-side array for easy logic checks, but use a **Shader-based Vertex Displacement** for rendering. Sync the state once per frame using a small texture update.

---

## 5. Real-Time Mesh Deformation

Even when using a **Static Mesh** on the GPU, you can implement real-time deformation (like craters or trenches) by treating your Heightmap Texture as a dynamic canvas.

### **The "Canvas" Approach**
The GPU's vertex buffer remains static (the geometric structure is constant), but the **Vertex Shader** displaces those vertices based on the current state of the texture. To "deform" the world:

1.  **CPU-Side Update:** When an event occurs (e.g., an explosion at world coordinates `x, z`), modify the corresponding height values in your RAM-resident array (e.g., `gTerrain`).
2.  **Partial Texture Update:** Use `glTexSubImage2D` (or equivalent) to upload only the modified block of height pixels to the GPU.
3.  **Frame Execution:** The Vertex Shader, which runs every single frame, samples the updated texture and instantly displaces the vertices to their new lower (crater) or higher (mound) positions.

### **Limitations & Constraints**
*   **Vertical Displacement Only:** This method is restricted to 2.5D. You cannot create overhangs, vertical tunnels, or caves. The mesh is always a single 2D plane projected into 3D space.
*   **Topological Continuity:** The mesh remains a continuous sheet. You cannot "tear" the ground into separate floating chunks or create holes without discarding fragments in the shader based on an alpha map.
*   **Normals & Lighting:** If you deform the ground, the pre-calculated normals will be wrong. In a modern shader system, you should calculate the surface normal in the **Fragment Shader** by sampling the heights of neighboring pixels to ensure lighting reacts correctly to the new terrain shape.

---

## 6. Flexible Dimensions & Formats

To move beyond the hardcoded $256 \times 256$ limitation, the system can be adapted to support arbitrary image dimensions and high-fidelity formats.

### **Formats**
*   **8-bit (Standard BMP/JPG/PNG):** Limits elevation to 256 discrete "steps". This can result in "staircase" artifacts on steep slopes.
*   **16-bit (PNG/TIFF):** Provides 65,535 steps, offering professional-grade smoothness for natural landscapes.
*   **Floating Point (EXR/HDR):** Recommended for "Any Format" support. It allows for negative heights and extremely high peaks without scaling hacks.

### **Dimension Constraints**
While modern hardware is flexible, the following constraints should be observed for a seamless experience:

| Constraint | Reason |
| :--- | :--- |
| **Power-of-Two (POT)** | Textures like $1024 \times 1024$ or $2048 \times 2048$ are required for hardware-level `GL_REPEAT` wrapping on some systems (especially mobile/older GLES). |
| **Aspect Ratio Awareness** | If the map is $512 \times 1024$, the wrapping logic must use separate `SIZE_X` and `SIZE_Z` constants. |
| **Memory limits** | A $4096^2$ map with a 32-bit `Terrain` struct would require **64MB** of CPU RAM and **64MB** of VRAM. |
| **Filtering** | When using non-integer world coordinates, ensure the texture is set to `LINEAR` filtering to avoid blocky edges between data samples. |

### **Implementation Proposal**
1.  **Dynamic Allocation:** Replace the fixed global array `gTerrain[SIZE][SIZE]` with a heap-allocated pointer: `Terrain *gTerrain = malloc(width * height * sizeof(Terrain));`.
2.  **UV Mapping:** Instead of tile indices, use normalized coordinates $(0.0$ to $1.0)$ in the shader. 
    `uv = worldPos.xz / mapSize_Units;`
3.  **Bilinear Override:** If not using shaders, your `TerrainGetHeight` function must be updated to use the dynamic `width` and `height` for index calculations: `index = (z * width) + x;`.

---

## 7. Memory Optimization & Struct Packing

The current `Terrain` struct is ~64 bytes per tile. While fine for $256^2$ (4.2MB), it becomes a bottleneck at $4096^2$ (1GB+). To scale up, you must "pack" your data.

### **The 32-bit "Packed Pixel" Strategy**
Instead of a large struct, represent each tile as a single `uint32_t`. This allows for a $4096^2$ map to fit in only **64MB** of RAM.

**Example Bit-Packing Layout:**
| Bit Range | Field | description |
| :--- | :--- | :--- |
| **0-7** | **Height** | 8-bit elevation (0-255). |
| **8-13** | **Object ID** | Supports 64 unique ground object types. |
| **14-17** | **Biome ID** | Supports 16 unique biomes. |
| **18-19** | **Status** | 0=Normal, 1=Infected, 2=Destroyed. |
| **20-29** | **Yaw** | 10 bits for rotation (enough for $\approx 0.35^\circ$ precision). |
| **30-31** | **Flags** | Hidden/Radar-jammed, etc. |

### **Separation of Concerns (SoA - Structure of Arrays)**
To keep the renderer fast while allowing for gameplay logic:
1.  **Static Data Array:** A compact `uint32_t` array containing the metadata above. This is what you upload to the GPU.
2.  **Dynamic Logic Map:** Use a **Sparse Map** (like a Hash Map or a Pool) for tiles that are currently evolving (e.g., a tree that is currently swaying or a tile being infected). 99% of tiles in a massive world are idle; they don't need to store `objectSway` or `objectCycle` until the player is nearby.

### **Procedural Visuals**
Avoid storing colors (`r, g, b`) in the struct. Instead, pass a **Palette Texture** to the Fragment Shader.
*   **The Shader:** Uses the `Biome ID` to sample a row in the palette texture.
*   **The Result:** You can change the "look" of the entire world (e.g., from Summer to Winter) just by swapping the palette texture, without touching the millions of terrain tiles in memory.
