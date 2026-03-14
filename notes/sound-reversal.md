Good clarification — "only reversed on the Z axis" is the key. This is a coordinate system porting issue. Let me look at how ships are drawn and the full bearing calculation to pinpoint the Z-axis confusion:

Searched for "DrawModel\|DrawModelEx\|RotateEntity\|ship.*yaw\|yaw.*ship\|DrawShip"
Searched for "DrawModel\|ship.*draw\|draw.*ship\|gShips\[.*\].*Draw"
Searched for "DrawModel"
Viewed main.c:1-498

This is the key insight. Look at the **orientation indicator** in [main.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:0:0-0:0) lines 152-157:

```c
float fx =  sinf(yr) * cosf(pr);
float fz =  cosf(yr) * cosf(pr);
// ...
Vector3 end = { drawPos.x + fx * al, ..., drawPos.z - fz * al };  // <-- Z is NEGATED
```

And the **ship rotation** at line 129: `rlRotatef(-drawYaw, 0, 1, 0)` — negative yaw applied.

So in the actual world/render space, the ship's **forward direction is `-Z`** at yaw=0 (because rlRotatef negates yaw, and the model's nose points toward -Z). The bearing calculation uses `atan2f(lx, lz)` which treats **+Z as forward** — but the player's actual facing is **-Z**. This is the Z-axis flip from the left-handed→right-handed port.

The fix: negate the `lz` component in `atan2f` across all three pan calculation sites in [ship.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c:0:0-0:0), since the player's actual forward in world space is `-Z` not `+Z`:
- `atan2f(lx, lz)` → `atan2f(lx, -lz)`

This makes "directly ahead" (lz negative) give bearing=0°, "directly behind" give bearing=180°, and left/right remain correct.

Viewed ship.c:160-172
Edited ship.c
Ran command: `make 2>&1 | tail -10`

Clean build. Here's the full explanation of what was happening and why:

---

## Root Cause: Left-Handed → Right-Handed Z-Axis Flip

In **Blitz3D** (left-handed coordinate system), **+Z = forward** (into screen). The original bearing calculation `atan2f(lx, lz)` was correct there: a sound at lz>0 (forward) gives bearing=0, which is centre-pan.

In **Raylib** (right-handed), the port flips Z in the mesh loader (`z = -z` in [my3d.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/my3d.c:0:0-0:0)) and the render code confirms the ship's actual forward is **-Z**:
- Ship draw: `rlRotatef(-drawYaw, 0, 1, 0)` — yaw negated so model faces -Z at yaw=0
- Orientation indicator: `end.z = drawPos.z - fz * al` — the facing line goes in **-Z**  
- Integration: `p->z = p->z - p->vz * 0.5f` — thrust moves in -Z

But `atan2f(lx, lz)` still used **+Z** as its reference "forward", so it was **180° off**. This caused front/back to be swapped in the bearing, and for diagonal sounds (e.g. front-right vs rear-left) to get their left/right pan values swapped.

**Fix:** `atan2f(lx, -lz)` in all three pan sites — negating `lz` corrects the reference axis to match the actual -Z forward convention, so the bearing angle is 0° when a sound is directly ahead and panning is correct throughout.