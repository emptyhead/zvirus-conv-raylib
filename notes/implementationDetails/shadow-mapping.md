I have completed the shadow system implementation and exposed all tuning variables to [src/constants.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h:0:0-0:0).

### 🛠️ Configuration Variables
You can now adjust the shadow system directly in [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L217-L224):
*   `SHADOW_MAP_SIZE`: Resolution of the shadow texture (e.g., 1024, 2048).
*   `SHADOW_CUBE_SCALE`: Adjusts the thickness of particle shadows (currently `1.2f`).
*   `SHADOW_CAMERA_FOV`: Vertical/Horizontal coverage of the light camera.
*   `SHADOW_BIAS`: Prevents "shadow acne" artifacts (default `0.0002f`).
*   `SHADOW_INTENSITY`: Transparency of the shadows (`0.9f` = 90% darkening).
*   `SHADOW_PCF_SAMPLES`: Controls shadow edge smoothness (0 = hard edges, 1 = 3x3, 2 = 5x5).

---

### 📝 Implementation Details

#### 1. Core Rendering Pipeline ([main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c))
*   **Initialization (L458-L519)**: Loads the custom shaders and initializes a `RenderTexture2D` with manual depth attachment support for sampling in shaders.
*   **Pass 1: Depth Generation (L103-L170)**: 
    *   Positions `gLightCamera` above the player looking down.
    *   Renders Ships (L117), Terrain Objects (L162), and Particles (L165) into the `gShadowMap`.
    *   Ships and Terrain Objects use a specialized `gShadowShader` which only writes depth.
*   **Pass 2: Main Rendering (L171-L400)**:
    *   Activates `gTerrainShadowShader` for the terrain mesh.
    *   Binds the shadow map to texture unit 1 and passes the `matLightVP` (Light View-Projection matrix) to the shader.

#### 2. Specialized Terrain Rendering ([terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c))
*   **[TerrainRenderObjectsEx](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:400:0-526:1) (L398-L516)**: I added this extended function to allow rendering static objects (trees, buildings) with an override shader. This ensures the first pass can efficiently capture their silhouettes for shadow generation.

#### 3. Particle Shadow Volumetrics ([particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c))
*   **[ParticleDrawAllEx](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:184:0-232:1) (L183-L229)**: Standard billboarded particles are thin and don't cast shadows when viewed from above. To fix this, I implemented a conditional path:
    *   **Main Pass**: Billboards face the camera for visuals.
    *   **Shadow Pass**: Renders small **Cubes** at the particle positions. This gives them "volume" so they cast distinct shadows regardless of the light angle.

#### 4. Shadow Calculation Shaders ([resources/shaders/](file:///home/pes/dev/C/zvirus-conv-raylib/resources/shaders/))
*   **[terrain_shadow.fs](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/resources/shaders/terrain_shadow.fs:0:0-0:0)**: Implements **PCF (Percentage Closer Filtering)** by sampling a 3x3 grid around each fragment to create smooth, anti-aliased shadow edges.
*   **`terrain_shadow.vs`**: Projects terrain vertices into the light's coordinate system, allowing the fragment shader to perform the depth comparison.

The system is fully integrated and optimized for the 60FPS target, providing a significantly more immersive depth-to-terrain interaction.

Edited constants.h
Viewed main.c:15-35
Edited main.c
Edited terrain_shadow.fs
Ran command: `make clean && make`
Edited shadow-mapping.md

I have implemented the dynamic PCF (Percentage Closer Filtering) sampling control and exposed it to [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L224).

### 🛠️ New Configuration Variable
*   `SHADOW_PCF_SAMPLES`: Controls the smoothness of shadow edges.
    *   `0`: Hard edges (single sample, fastest).
    *   `1`: 3x3 grid (9 samples, balanced).
    *   `2`: 5x5 grid (25 samples, softest, most demanding).

### 📝 Final Implementation Summary
*   **Shader Enhancement**: Updated [terrain_shadow.fs](file:///home/pes/dev/C/zvirus-conv-raylib/resources/shaders/terrain_shadow.fs) to use a dynamic loop based on the `shadowSamples` uniform. It now supports variable kernel sizes for PCF.
*   **Infrastructure**: Added uniform plumbing in [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c) to pass the sample count from the C constants to the GPU.
*   **Documentation**: Updated the [Shadow Mapping technical notes](file:///home/pes/dev/C/zvirus-conv-raylib/notes/implementationDetails/shadow-mapping.md) with the new parameter.

The game is built and ready. You can now tweak the shadow quality vs. performance balance directly from the constants file!