# Environment Particles & Rain Logic Fix

This document outlines the changes made to the environment particle constants and the underlying terrain logic to ensure the rain frequency correctly reflects the infection progress, matching the original Blitz3D behavior.

## Core Change Summary

Previously, the rain system appeared unresponsive to the `G_RAIN_BIAS` constant because the infection stats were being calculated incorrectly. Sea tiles (Index 0) and Alien Buildings (Index 4) were being counted as part of the `gAreaInfected` pool during map load, causing the rain probability to hit near-maximum immediately upon starting a level.

### 1. Constant Documentation & Refinement
Line range: [src/constants.h:L213-230](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L213-230)

We added descriptive comments and examples for all environment constants. The `G_RAIN_BIAS` was identified as a subtractive probability threshold.

*   **Logic**: `shouldRain = (Rand(0, gAreaTotal) - G_RAIN_BIAS) < (gAreaInfected * G_RAIN_FREQ_SCALE)`
*   **Effect**: A higher bias (e.g., 5000) makes rain extremely frequent regardless of infection, while a lower bias (e.g., 50) relies on infection progress to trigger rain.

### 2. Terrain Statistic Fix (Correcting `gAreaTotal` and `gAreaInfected`)
Line range: [src/terrain.c:L192-215](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#L192-215) and [src/terrain.c:L252-286](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#L252-286)

In the C port, we were initially using `SIZE * SIZE` for the total area. We corrected this to match the Blitz3D logic where only "infectable land" (biomes 1, 2, and 3) contributes to the total pool.

**C Implementation Fix:**
```c
// Track total infectable land (Indices 1, 2, 3)
if (t->landIndex > 0 && t->landIndex < 4) {
    gAreaTotal++;
}
```

We also removed the automatic increment of `gAreaInfected` for sea and buildings during the coloring pass, as these should be considered "visually" infected but not part of the gameplay progress metric.

## Original Source Parity

The following references from the original `Source.bb` were used to verify the correct behavior:

*   **Rain Spawning Logic**: [Source.bb:L1668](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb#L1668)
    > `No = ( 1+ GridSize * 2.0 ) * ( ( Rand( AreaTotal ) -50 ) < AreaInfected * 4.0 )`
*   **Total Area Calculation**: [Source.bb:L1910](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb#L1910)
    > `AreaTotal = AreaTotal + ( terrain(x,z)\LandIndex > 0 And terrain(x,z)\LandIndex < 4 )`

## Result
The project now correctly scales rain frequency based on how much of the "infectable" land has been taken over. The `G_RAIN_BIAS` constant now acts as a meaningful slider for the "base" raininess of the environment.

With these changes, G_RAIN_BIAS will now correctly determine the "base" chance of rain at 0% infection.

At 50.0, rain will be extremely rare at the start of a level.
At 5000.0, you will see frequent rain bursts even before the infection spreads.