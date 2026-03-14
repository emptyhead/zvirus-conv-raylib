I have successfully exposed the player and system control bindings to [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h) and refactored the codebase to use these constants. This allows for easy reconfiguration of mouse and keyboard inputs across the entire application.

### Key Implementation Details:

1.  **Centralized Keybinds**: Added a comprehensive list of `BIND_*` macros to [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h):
    *   **Combat**: Firing, Thrusting, and Launching (supporting both Keyboard and Mouse buttons).
    *   **Steering**: Yaw and Pitch controls (WASD by default).
    *   **System**: Camera toggle, Pause, Exit, and debug controls (Grid size, Time scale).
    *   **Menu**: Navigating and selecting in menus.
2.  **Logic Integration**:
    *   Updated [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) to use the new combat and steering bindings. Firing and Thrusting now check both their assigned key and mouse button.
    *   Updated [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c) to handle system-wide keys (like Pause or Exit) through the definitions in [constants.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h:0:0-0:0).
    *   Updated [menu.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/menu.c) to allow configurable menu navigation keys.
3.  **Safety Checks**: Added logic in the input checks (e.g., `BIND_THRUST_MOUSE >= 0`) to ensure that if a binding is disabled (set to `-1`), the game won't trigger an unintentional input or crash.

### Refined Control Bindings in [constants.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h:0:0-0:0):

```c
// --- Keybinds ---
#define BIND_FIRE_KEY        KEY_SPACE
#define BIND_FIRE_MOUSE      MOUSE_BUTTON_LEFT
#define BIND_THRUST_KEY      KEY_X
#define BIND_THRUST_MOUSE    MOUSE_BUTTON_RIGHT
#define BIND_LAUNCH_KEY      KEY_M
#define BIND_LAUNCH_MOUSE    -1 // No default mouse bind

#define BIND_YAW_LEFT_KEY    KEY_A
#define BIND_YAW_RIGHT_KEY   KEY_D
#define BIND_PITCH_UP_KEY    KEY_W
#define BIND_PITCH_DOWN_KEY  KEY_S

#define BIND_CAMERA_KEY      KEY_C
#define BIND_PAUSE_KEY       KEY_P
#define BIND_EXIT_KEY        KEY_Q
...
```