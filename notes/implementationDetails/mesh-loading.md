# Mesh Loading Implementation: Blitz3D to C/RayLib

This document details the porting of the proprietary `.my3d` mesh format and entity rendering system from the original Blitz3D source to C and RayLib.

## 1. Original Source Mapping (Source.bb)

The following functions and logic blocks from `Source.bb` were analyzed and ported:

*   **MESHload (Line 2142)**: The core binary loader.
    *   Ported logic: Reading surface counts, vertex data (pos, normal, color, UV), and triangle indices.
    *   Coordinate Correction: Blitz3D is left-handed (Z forward). RayLib is right-handed (Z backward). We inverted the Z-axis in vertices and flipped the triangle winding order to maintain CW culling.
*   **GameObjectData (Line 2888)**: Data tables for mesh assignments.
    *   Mapped string names for `GroundObject` (Line 2891) and `FlyingObject` (Line 2894).
    *   Case-sensitivity handled for Linux: `tree1`, `house1`, etc.
*   **TERRAINupdate (Lines 1743–1755)**: Object rendering and animation.
    *   `Radar dish rotation`: Ported from `rotate` variable update (Line 1681).
    *   `Windmill blades`: Ported manual offset `20/16` (Line 1348) and rotation `rotate * 5` (Line 1760).
*   **Animation Clock (Line 1681)**:
    *   Blitz: `Rotate = WRAP ( Rotate + 2 , 360 )` at 30Hz.
    *   C/RayLib: Interpolated `gRotate` in `main.c` to maintain speed at 60Hz.

## 2. C Implementation Details

### Custom Loader (src/my3d.c)
A custom binary parser was written to handle the `.my3d` format since it is not natively supported by RayLib. It converts the binary data into RayLib `Mesh` and `Model` structures.

### Mesh States (MESHchange Port)
The system now supports three visual states for every terrain object, pre-loaded during initialization:
1.  **Normal**: Original vertex colors.
2.  **Infected**: Vertex colors shifted towards purple/magenta using the `(v + 250) * 0.5` formula from `Source.bb:2211`.
3.  **Destroyed**: Darkened colors and vertex deformation. 
    *   `Implode`: Scaled to `(0.5, 0.75, 0.5)` for smaller objects.
    *   `Squash`: Scaled to `(1.0, 0.5, 1.0)` for houses and larger structures.

### Dynamic Bounding Boxes (src/terrain.c)
Unlike the original hardcoded heights, the C port now uses `GetModelBoundingBox` (Line 364) to automatically calculate collision heights for houses and buildings based on the `meshState[0]` geometry. This fixed an issue where placeholder collision boxes were taller than the visual house meshes.

### Composite Models
Ground objects now support a `meshExtra[3]` array in the `GroundObject` struct (src/world.h) to handle secondary components that rotate independently across all states:
*   **Radar基地 (Radar1.my3d)** + **Radar Dish (Radar2.my3d)**
*   **Windmill Base (Mill1.my3d)** + **Windmill Blades (Mill2.my3d)**

## 3. Coordinate Parity
*   **Scaling**: All meshes are scaled by `OSCALE` (0.0625f) during loading to match the world units.
*   **Positioning**: Handled via `rlPushMatrix`/`rlPopMatrix` to apply base terrain heights (`landHeight`) and independent rotation axes for blades and dishes.
