I've implemented the **Fade** feature as requested, ensuring it matches the original Blitz3D behavior while adapting it for RayLib's 60Hz timing and C project structure.

### Implementation Summary

1.  **Global State ([world.h](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/world.h:0:0-0:0)/[world.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/world.c:0:0-0:0))**:
    *   Added `float gFadeStatus` to track the current opacity. Following the original Blitz logic:
        *   `0.0` = Full Black (Opaque)
        *   `1.0` = Clear (Transparent)
    *   This global state allows both the HUD and the transition logic to stay in sync.

2.  **HUD Overlay ([hud.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c:0:0-0:0))**:
    *   Modified [HudDraw](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c:51:0-116:1) to render a black overlay across the entire screen whenever `gFadeStatus` is less than `1.0`.
    *   Used RayLib's [Fade(BLACK, alpha)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:214:0-241:1) to correctly apply the transparency based on `1.0 - gFadeStatus`, perfectly mirroring the Blitz `EntityAlpha` behavior.

3.  **Blocking Transitions ([main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0))**:
    *   Implemented [HudFade(GameContext *g, float target, float speed)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:214:0-241:1) in [main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0).
    *   **Blocking Behavior**: To match the original Blitz code, this function runs its own rendering loop ([DrawFullFrame](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:72:0-212:1)), allowing the game to "pause" everything except the visual fade and rendering.
    *   **60Hz Calibration**: The `speed` parameter is automatically scaled by `30 / FPS`. If you pass the original Blitz speed of `0.05`, it correctly calculates `0.025` per frame at 60Hz to maintain the same real-world duration.
    *   **Refactoring**: Extracted all rendering logic into [DrawFullFrame](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:72:0-212:1) and promoted `terrainModel` to a global `gTerrainModel` so that [HudFade](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:214:0-241:1) can redraw the scene accurately during transitions.

4.  **Integration Points**:
    *   **Boot**: Added a fade-in sequence at the very start of the app (matches `Source.bb:410`).
    *   **Death**: Integrated fades into the player death logic (matches `Source.bb:1523`). The screen now fades to black before the player is reset and then fades back in.
    *   **Wave Transitions**: Added fades to the wave end and start sequences (matches `Source.bb:437` and `GAMEstart`).

### Code Changes

#### [world.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/world.h)
```c
extern float gFadeStatus; // 0.0=Black, 1.0=Clear
```

#### [hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c)
```c
// Inside HudDraw
if (gFadeStatus < 1.0f) {
    float alpha = 1.0f - gFadeStatus;
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, alpha));
}
```

#### [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c)
```c
void HudFade(GameContext *g, float target, float speed) {
    float perFrame = speed * (30.0f / (float)g->fps);
    while (gFadeStatus != target && !WindowShouldClose() && !g->flags.quit) {
        gFadeStatus = clampf(gFadeStatus + perFrame, 0.0f, 1.0f);
        // Clamp to target to prevent overshoot
        if (speed > 0 && gFadeStatus > target) gFadeStatus = target;
        if (speed < 0 && gFadeStatus < target) gFadeStatus = target;
        DrawFullFrame(g);
    }
}
```

This completes the `FADE` feature as noted in [MIGRATION_STATUS.md](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/notes/MIGRATION_STATUS.md:0:0-0:0). You should now see smooth black-to-clear and clear-to-black transitions when starting waves or respawning after death.