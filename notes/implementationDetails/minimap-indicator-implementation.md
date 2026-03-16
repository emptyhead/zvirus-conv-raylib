# Player Minimap Heading Indicator Implementation

Analysis of the player's heading indicator on the HUD minimap, comparing the original `Source.bb` implementation with our Raylib C port.

## Implementation Overview

The player indicator consists of the traditional colored dot combined with a translucent "view wedge" triangle. This wedge originates from the dot and rotates with the ship to represent the player's heading/view range.

### Files Involved

| Project File | Original Source (`Source.bb`) | Description |
| :--- | :--- | :--- |
| `src/hud.c` (lines 112-140) | `Source.bb` (lines 627-630, 847-854) | Indicator drawing and coordinate mapping |
| `src/constants.h` (lines 35-42) | `Source.bb` (Global definitions) | Configuration constants for size/color |

---

## Comparison of Implementations

### 1. Indicator Type
- **Original (`Source.bb`)**: Uses a 3D mesh (`HUD\arrow`) loaded from `TestHud` assets. It snaps the arrow to the player's dot position on the HUD and rotates it.
- **Raylib Port**: Uses a procedural `DrawTriangle` to create a "view wedge" that originates from the player's dot. This represents the player's field of vision/heading while maintaining the traditional dot indicator. Both the dot and wedge are now fully configurable via constants.

### 2. Drawing Logic
- **Original**: Parented `HUD\arrow` to the player's minimap dot and applied rotation.
- **Raylib Port**:
  1. Draws a triangle (the view wedge) with its first vertex at the player's minimap center `(mx, my)`.
  2. The outer vertices are calculated using the ship's `yaw` and a `wedgeAngle`.
  3. Draws the player's colored dot on top of the triangle origin.

### 3. Coordinate Mapping
- **Original**:
  ```blitzbasic
  mapx# = -246  + ( p\x -128.0 )  *.5
  mapy# = 166 + (128.0 - p\z ) * .5
  PositionEntity p\dot, mapx,mapy,0
  ```
  The original HUD uses a dedicated 3D camera (`HUD\camera`) with fixed offsets.
- **Raylib Port**: Normalized world coordinates mapped to the minimap pixel space.

### 4. Rotations and Heading (CCW Support)
Our port uses an inverted Z-axis and Counter-Clockwise (CCW) rotations. To account for this, the wedge vertices are calculated as:

```c
// Origin Point
Vector2 v1 = { mx, my };
// Outer points for CCW winding (v2: +wedgeAngle, v3: -wedgeAngle)
Vector2 v2 = { mx + sinf(angleRad + wedgeAngle) * viewLen, my - cosf(angleRad + wedgeAngle) * viewLen };
Vector2 v3 = { mx + sinf(angleRad - wedgeAngle) * viewLen, my - cosf(angleRad - wedgeAngle) * viewLen };
```

---

## Variances and Adjustments

### Update Rate (60Hz vs 30Hz)
- While the **original runs at 30Hz**, our port runs at **60Hz**.
- **Impact**: The UI drawing itself doesn't require interpolation for a static HUD, but the smoothness of the indicator's movement is doubled.

### Coordinate System
- **Original**: Z-axis is positive forward. Minimap Y uses `128 - p\z`.
- **Raylib Port**: Z-axis is inverted (standard for our implementation).
- **Rotation**: Positive rotation increases CCW. The use of `-sinf(angleRad)` in the vertex calculation ensures the triangle points correctly.

### Configuration Constants (src/constants.h)
- `MINIMAP_DOT_PLAYER`: Size of the dot representing the player.
- `MINIMAP_VIEW_SIZE`: Length of the view indicator wedge.
- `MINIMAP_VIEW_COLOR_R/G/B`: RGB components for the wedge color.
- `MINIMAP_VIEW_ALPHA`: Transparency of the wedge (0-255).
