Searched for "lives"
Searched for "explosion"
Viewed game.h:1-47
Viewed ship.c:1-744
Searched for "lives--"
Searched for "lives -="
Viewed main.c:1-733
Searched for "p\dead = 1"
Viewed Source.bb:1151-1950
Searched for "dead = 1"
Viewed Source.bb:2250-2350
Viewed Source.bb:800-1000
Searched for "deadTimer ="
Viewed world.c:1-213
Viewed camera_game.c:1-81
Edited main.c
Viewed particle.c:1-234
Viewed score_tag.c:1-109
Viewed constants.h:1-312

The issue you're experiencing is caused by a race condition between the high-speed simulation loop (which can run multiple physics steps per frame) and the game's death-handling logic. 

### The Problem
*   **Missing Explosion:** Most of the time, the player's death is detected, and the game immediately triggers a "fade and reset." Because the reset happens so fast, the [ShipUpdateAll](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c:475:0-733:1) function never gets a chance to see the `dead == 1` state in a simulation step, so it never triggers the explosion particles or sound.
*   **Missing Reset (Hang):** Occasionally, if the simulation runs two steps in a single frame, the first step sets the player to `dead = 1`, and the second step immediately triggers the explosion and moves the player to `dead = 2`. The game's main loop was only looking for `dead == 1` to trigger a restart, so it would miss the window and the game would continue without a player.

### The Fix
I have implemented the following improvements:
1.  **Robust Death Detection:** Changed the check in [main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0) to look for `dead >= 1`. This ensures that even if the explosion has already been triggered in the simulation loop, the game still proceeds to the life-loss and reset sequence.
2.  **Forced Explosion Trigger:** If the player has died but the explosion hasn't triggered yet, we now force one logic update to ensure the sound and particles are generated before the fade starts.
3.  **Animated Fades:** Updated the blocking [HudFade](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:397:0-434:1) loop to continue updating particles, score tags, and the camera. This ensures that the explosion actually "explodes" and animates during the fade-to-black, rather than appearing as a static burst.

### Changes Summary

#### [src/main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0)
- **Death Sequencing**: Modified the logic in `STATE_WAVE` to handle any death state `≥ 1`. It now explicitly triggers a logical update if the ship just died (`dead == 1`) to ensure the explosion particles and audio are created.
- **Fade Animation**: Updated [HudFade](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:397:0-434:1) to call [ParticleUpdateAll()](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:128:0-178:1), [ScoreTagUpdateAll()](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c:51:0-66:1), and [CameraGameUpdate()](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/camera_game.c:24:0-79:1) within its blocking loop. This keeps the game world looking "alive" and moving while the screen fades.

You can now test the game. When you crash or are destroyed, you should consistently see/hear an explosion followed by the fade and player restart.

render_diffs(file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c)