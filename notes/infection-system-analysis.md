# Infection System Analysis

Analysis of discrepancies between the original Blitz3D `Source.bb` infection system and the C/RayLib port. Original runs at 30Hz; C port runs at 60Hz.

---

## How Infection Spreads (Both Versions)

There are three mechanisms:

### 1. On-screen Seeder/Bomber (particle drop)
- Seeder (AI 1) emits particle template 11 every `fireRate` frames when `reload == 0`.
- Bomber (AI 2) emits particle template 12 similarly.
- Particles fall under gravity. On ground contact they call `MAPadd / TerrainMapAdd`:
  - ID 10 (tree drip) → `ss = 1` (3×3 cell area)
  - ID 11 (seeder spore) → `ss = 2` (5×5 cell area)
- Gated by `reload = fireRate` after each shot. ✅ Correctly ported.

### 2. Off-screen Seeder/Bomber (direct map write)
- When a Seeder or Bomber is outside the visible grid, it skips particle spawning and instead writes directly to the terrain map via `MAPadd` at a random offset (radius 0–6 cells).
- **Original (Source.bb ~line 817):** no reload assignment in this branch — it fires every frame at 30Hz.
- **C port:** also fires every frame, but at 60Hz → **2× the original rate**. *(Bug — now fixed.)*

### 3. Tree drip (infected object emission)
- Infected trees/bushes (`objectStatus == 1`, `objectIndex < 5`) emit particle ID 10 every `objectCycle` ticks.
- Original cycle: `wrap 20` at 30Hz (emits once per ~0.67s).
- C port cycle: `% 40` at 60Hz (emits once per ~0.67s). ✅ Correctly adjusted.

---

## Discrepancies Found

### 🔴 Off-screen infection rate (Critical — **Fixed**)

| | Original (30Hz) | C Port (60Hz, before fix) | C Port (60Hz, after fix) |
|---|---|---|---|
| Fires per second | ~30 | ~60 | ~30 equivalent |
| Mechanism | No reload throttle, fires every frame | Same — but 2× as often | Throttled by `fireRate` |

**Root cause:** The off-screen branch in `Source.bb` omits any `p\Reload` assignment — it fires every frame. At 30Hz that is acceptable. The C port inherited the same omission but runs at 60Hz, doubling the infection rate for off-screen entities.

**Fix applied (`ship.c` line ~716):**
```c
TerrainMapAdd(0, p->x + ra * cosf(an), p->z + ra * sinf(an), 1);
p->reload = F->fireRate;  // throttle to fire-rate cadence (30Hz parity)
```

### 🟡 Infected colour mutation (Cosmetic — not fixed)

**Original `MAPadd`** permanently overwrites the uninfected colour with the infected colour when a cell is first infected:
```blitzbasic
t\r[0] = t\r[1]
t\g[0] = t\g[1]
t\b[0] = t\b[1]
t\LandInfected = 1
```
The terrain vertex colouring then always reads `r[0]` unconditionally.

**C port `TerrainMapAdd`** does not copy the colour arrays. Instead `TerrainUpdateGrid` uses a runtime index:
```c
int colorIdx = (t->landInfected > 0) ? 1 : 0;
```
This is functionally equivalent for normal gameplay (cells are never de-infected). It does not affect infection rate.

---

## What Was Correct

| System | Status |
|---|---|
| `ss` radius (`p->id - 9`) for ID 10 and 11 | ✅ Correct |
| On-screen seeder spore gating by `reload` / `fireRate` | ✅ Correct |
| `objectCycle` wrap at 40 (vs Blitz's 20) for tree drip rate | ✅ Correct |
| `gAreaInfected` counter increment | ✅ Correct |
| Initial water/alien-building cells pre-infected on level load | ✅ Correct |

---

## Relevant Source Locations

| Item | Blitz Source | C Port |
|---|---|---|
| Off-screen infection | `Source.bb:816–820` | `ship.c:712–719` |
| On-screen spore spawn | `Source.bb:809–811` | `ship.c:669–677` |
| Particle ground-hit infection | `Source.bb:2314` | `particle.c:158–160` |
| Map infection write | `Source.bb:1580–1602` | `terrain.c:TerrainMapAdd` |
| Tree drip emission | `Source.bb:1776` | `terrain.c:TerrainUpdateObjects` |
