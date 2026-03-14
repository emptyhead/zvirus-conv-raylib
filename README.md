## zvirus-conv-raylib

Incremental C port of the legacy Blitz3D project (`Source.bb`) using raylib.

### Build

Requires raylib installed system-wide with `pkg-config` metadata.

```bash
make
```

### Run

```bash
make run
```

### Controls (current skeleton)

- **Flight Controls:**
  - `WASD` or `Arrow Keys`: pitch/yaw (turn) - yellow arrow shows direction
  - `Space`: thrust (forward acceleration)
- **Game Controls:**
  - `Enter`: start game / advance to next wave
  - `=`: mark wave complete (debug)
  - `Q`: quit

### Features (current)

- ✅ Terrain rendering (continuous mesh instead of debug cubes)
- ✅ Terrain objects (scattered colored cubes representing bushes/houses)
- ✅ Player movement (pitch/yaw with momentum, space bar thrust)
- ✅ **Enhanced Enemy AI** with 3 distinct behaviors:
  - 🔵 **Blue: Seeders** - Patrol and search for targets
  - 🟢 **Green: Bombers** - Fly high, dive when attacking
  - 🟣 **Purple: Spores** - Erratic, unpredictable movement
- ✅ Visual orientation feedback (yellow arrow shows player facing direction)
- ✅ World wrapping (fly off one edge, appear on opposite side)

