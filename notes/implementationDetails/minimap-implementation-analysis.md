# Minimap & Radar Implementation Analysis

This document details the transition of the Minimap and Entity Radar systems from the original BlitzBasic ([Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb)) to the ported C/Raylib implementation.

## 1. System Overview

The minimap consists of two layers:
1.  **Static/Background (The Map)**: A 256x256 texture representing the terrain and infection state.
2.  **Dynamic/Foreground (Entity Radar)**: Real-time dots representing the player, allies, and enemies.

## 2. File & Line References

| Feature | BlitzBasic ([Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb)) | C/Raylib Project |
| :--- | :--- | :--- |
| **Data Structure** | `HudType` (Lines 192-207) | `Hud` ([world.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/world.h#L124-L139)) |
| **Initialization** | `PLAYERreset` (Lines 571-578) | `HudInit` ([hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#L12-L16)) |
| **Map Texture Update** | `MAPupdate` (Lines 1628-1640) | `HudUpdateMapIncremental` ([hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#L52-L72)) |
| **Entity Visualization** | `PLAYERupdate` (Lines 846-854) | `HudDraw` loop ([hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#L105-L146)) |

## 3. Implementation Comparison

### A. Coordinate Mapping & Visual Logic
In BlitzBasic, the radar dots were actual 3D entities parented to a HUD camera. The positioning used manual offsets into screen-space coordinates.

**Original Blitz Calculation:**
```bb
mapx# = -246 + (p\x - 128.0) * .5
mapy# = 166 + (128.0 - p\z) * .5
```

**Ported C Calculation:**
We use a 2D drawing approach with Raylib's `DrawCircle`, mapping the grid normalized to the map's visual size (128px).
```c
float px = (p->x / (float)SIZE) * (float)mapSize;
float pz = (p->z / (float)SIZE) * (float)mapSize;
float mx = (float)mapX + px;
float my = (float)mapY + pz;
```

### B. Variances & Adjustments

#### 1. Coordinate System (Inverted Z)
The original project treats **increasing Z** as "forward" or "upward" (away from the camera). In our 2D minimap projection:
*   **X (World)** maps to **X (Screen)**.
*   **Z (World)** maps to **Y (Screen)**.
*   **Variance**: While the Blitz code manually flips Z (`128.0 - p\z`), the C port aligns the world Z orientation directly with the screen Y axis for simplicity.

#### 2. Rotation Directions
The player's heading indicator (arrow/line) must account for the difference in rotation math:
*   **Blitz**: Used `RotateEntity HUD\arrow, 0, 0, player(cam)\yaw`.
*   **Raylib**: Uses standard trigonometry. `sinf(angleRad)` for X and `-cosf(angleRad)` for Y provides a "North-is-Zero" behavior where 0 degrees points up.

#### 3. Update Rates (60Hz vs 30Hz)
*   **Blitz (30Hz)**: The radar dots were 3D entities. Their visibility was toggled occasionally or updated within the `PLAYERupdate` loop.
*   **C (60Hz)**: The minimap is redrawn entirely every frame inside the 60fps render loop (`HudDraw`). 
*   **Benefit**: This leads to significantly smoother movement of dots across the radar, as they update every frame rather than following the 30Hz logic ticks.

#### 4. Visibility Logic
We strictly mirror the original's visibility quirks:
*   **The Monster (AI 17)**: Hardcoded to be invisible on radar (`EntityAlpha 0` vs `if (e->ai == 17) continue`).
*   **Radar Jammers**: Enemies on `landHidden` terrain tiles are hidden if they belong to the enemy faction (`index == 1`).

## 4. Entity Colors
Colors are pulled directly from the `FlyingObject` definitions (`F\r, F\g, F\b`), ensuring the radar matches the craft's primary color theme defined in `world.c`.
