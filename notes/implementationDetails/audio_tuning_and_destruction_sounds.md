# Audio Tuning and Ground Destruction Sounds

This document details the implementation of configurable sound volumes and the addition of ground object destruction sounds, aligning the C port more closely with the original BlitzBasic sound logic.

## Summary of Changes

### 1. Global and Group Volume Constants
**File:** [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h) (Lines 267-297)

Exposed a set of tuning constants to control audio volumes globally and per-group. This allows for fine-tuning the mix without digging into implementation files.

- `SND_VOL_MASTER`: Global gain multiplier.
- `SND_VOL_GROUP_0`: Boosted to `1.2f` to address the "quiet player engine" issue.
- `SND_VOL_SHOOT`: Standardized shooting volume (boosted to `0.8f`).
- `SND_VOL_SMALL`: Volume for ground object hits/destruction.
- `SND_FALLOFF_DIST`: Distance at which sounds become silent (standardized to 80 units).

### 2. 3D Audio Refactoring
**File:** [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) (Lines 148-171, 691-703)

- **`ShipPosition`**: Refactored the looping sound group logic. It now applies group-specific base volumes (e.g., boosting the player's `thrustloop`) and uses `SND_FALLOFF_DIST`.
- **`ShipUpdateAll`**: Updated shooting audio to use `SND_VOL_SHOOT`.

**Relation to Source.bb:**
- Mirrors `PLAYERposition` (`Source.bb:928-941`) where the engine sound volume is modulated by thrust and distant panning is calculated.

### 3. Ground Object Destruction Audio
**File:** [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) (Lines 640-650)

Previously, ground objects were destroyed silently. A 3D audio trigger was added to `TerrainCollisionGround` to play `gSoundSmall`.

- Calculates distance/pan relative to the camera for local feedback.
- Uses the `SND_VOL_SMALL` constant.

**Relation to Source.bb:**
- Corresponds to `PARTICLEupdate` (`Source.bb:2315`), where a check for bullet-to-ground collision plays `gSoundSmall`.

### 4. Standardized Ambient and UI Volume
**Files:** [audio.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/audio.c), [menu.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/menu.c)

- Standardized the ambient wind volume and menu "click" sounds to use the new constants (`SND_VOL_WIND`, `SND_VOL_MENU`).

## Impact on Gameplay
- The player's engine (Sound Group 0) is now clearly audible even at low thrust.
- Shooting feels more impactful due to boosted default volumes.
- Ground combat provides immediate audio feedback when destroying structures or trees, matching the original 1990s arcade feel of the Blitz project.
