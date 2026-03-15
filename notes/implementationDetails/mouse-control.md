The mouse delta-based pitch and yaw controls have been implemented, replacing the previous WASD/Arrow key bindings for rotation. The implementation includes sensitivity settings and an inversion flag for pitch in [src/constants.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h:0:0-0:0), as requested.

### Summary of Changes

*   **Configurable Sensitivity & Inversion**: Added `MOUSE_YAW_SENSITIVITY`, `MOUSE_PITCH_SENSITIVITY`, and `MOUSE_INVERT_PITCH` to [src/constants.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h:0:0-0:0).
*   **Mouse Delta Integration**:
    *   Captured mouse movement in the main loop ([src/main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0)) once per frame.
    *   Distributed the delta across simulation steps to ensure consistent rotation speed regardless of frame rate (e.g., if simulation is 60Hz and frame rate is 30Hz or 120Hz).
    *   Applied the delta to the player's yaw and pitch in [src/ship.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c:0:0-0:0), accounting for the internal steering logic and faction alignment.
*   **Cursor Management**: The mouse cursor is now automatically disabled and captured when in the `WAVE` state (gameplay) to allow for "infinite" rotation, and re-enabled in menus or pause screens.

### Files Modified

| File | Change Description |
| :--- | :--- |
| [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h) | Added mouse sensitivity and inversion constants. |
| [ship.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.h) | Declared global mouse delta variables for synchronization. |
| [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) | Replaced WASD rotation logic with mouse delta input. |
| [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c) | Implemented mouse delta distribution and cursor state management. |

### Configuration

You can adjust the behavior in [src/constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h#L218-L225):

```c
// --- Mouse Controls ---
static const float MOUSE_YAW_SENSITIVITY = 0.5f;
static const float MOUSE_PITCH_SENSITIVITY = 0.5f;
static const int MOUSE_INVERT_PITCH = 0; // 0 = normal, 1 = inverted
```

The project has been successfully rebuilt and is ready for testing.