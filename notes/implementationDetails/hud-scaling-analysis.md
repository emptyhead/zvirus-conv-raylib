# HUD Scaling & Configuration Analysis

This document details the implementation of the scalable HUD system and the centralization of HUD constants, comparing the modern C/Raylib approach with the original resolution-dependent scaling in BlitzBasic ([Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb)).

## 1. Objective of Changes
The goal was to move away from hardcoded pixel values in the HUD rendering and provide a centralized, scalable system (`HUD_SCALE`) that allows the entire UI (minimap, bars, text, and crosshair) to be adjusted without rewriting drawing logic.

## 2. File & Line References

| Feature | BlitzBasic ([Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb)) | C/Raylib Project |
| :--- | :--- | :--- |
| **Global Scaling Factors** | `HGW`, `HGH` (Graphic-based constants) | `HUD_SCALE` ([constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L30)) |
| **Minimap Scaling** | Line 578 (`ScaleEntity` with `HGW`) | `MINIMAP_SIZE` ([constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L33)) |
| **Fuel/Height Bars** | Lines 1436-1560 (Part of `GAMEupdate` UI) | `HUD_BAR_W`, `HUD_BAR_H` ([constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L38-L39)) |
| **HUD Rendering Logic** | `HUDupdate` (Lines 2552-2590) | `HudDraw` ([hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#L74-L164)) |

## 3. Implementation Comparison

### A. Scaling Methodology
In the original **BlitzBasic** code, scaling was often tied to the "Half Graphic Width/Height" (`HGW`, `HGH`). For example, radar dots were scaled using `HGW` to maintain a consistent relative size across different resolutions.
```bb
578: ScaleEntity p\dot, F\radius * HGW, F\radius * HGW, HGW 
```

In the **C Port**, we have introduced a centralized `HUD_SCALE` (currently 1.5x) and derived constants. This ensures that:
*   Visual hierarchy is preserved (e.g., text labels scale with the bars).
*   Dots on the minimap remain legible regardless of the overall UI size.
*   The crosshair preserves its relative impact on the screen center.

### B. Variances & Adjustments

#### 1. Coordinate System & UI Placement
*   **Blitz**: Relied on `HGW` for centering and `EntityParent` for HUD positioning (3D entities in front of camera).
*   **Raylib Port**: Uses 2D screen coordinates with implicit scaling. To handle high-precision center alignment (fixing the crosshair and bars), we moved to floating-point division (`sw / 2.0f`) before casting to `int` for drawing. This avoids the 1-pixel "shimmering" or misalignment common in integer-only HUD code.

#### 2. Update Rates & Input Sensitivity
*   **Original (30Hz)**: Original UI text was often updated on a 30Hz tick.
*   **Port (60Hz)**: All scalable elements (including the direction indicator and dots) are redrawn at 60Hz. The `MINIMAP_DIR_LEN` and `HUD_SCALE` work together to ensure the heading arrow moves smoothly at the higher refresh rate, providing immediate feedback for quick turns.

#### 3. Inverted Rotation & Z-Axis
*   **Rotation**: When scaling the ship heading indicator, we use `DrawLineEx`. In the Raylib port, `p->yaw` is used with `sinf` and `cosf`. Because screen-space Y is inverted compared to math-space Y (Y increases downwards), we subtract the cosine component: `my - cosf(angleRad) * dirLen`. Scaling `dirLen` by `HUD_SCALE` maintains the correct visual "reach" of the arrow on the radar.

## 4. Why the Change Was Needed
The original 128px minimap was designed for lower resolution displays. At modern resolutions, the radar dots (1px in the original) were nearly invisible. By increasing the HUD by 1.5x and exposing `MINIMAP_DOT_ENTITY`, we ensure the player can actually see the movement patterns of Pests and Fighters, which is critical for gameplay.
