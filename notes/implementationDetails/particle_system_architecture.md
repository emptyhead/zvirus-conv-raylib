# Particle System & Ground Interaction Architecture

This document describes the performant particle system in the `zvirus` project, covering its architecture, environmental effects, and ground interaction logic.

## 1. Core Architecture: Fixed-Size Pool

To maintain high performance and avoid memory fragmentation, the project uses a pre-allocated pool of particles.

- **Storage:** A global array `gParticles[MAX_PARTICLES + 1]` (max 16,384) in [world.c](file:///home/pes/dev/agent/zvirus-conv-raylib/src/world.c).
- **Allocation:** Uses a circular buffer pattern via `gParticleNext` in [particle.c](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#L100-L131). When the limit is reached, it wraps around to index 0, naturally recycling old particles.
- **Templates:** Each particle is governed by a `templateID` that defines behaviors like color range, velocity spread, fade rate, and gravity influence in [world.h:L43-L51](file:///home/pes/dev/agent/zvirus-conv-raylib/src/world.h#L43-51).

## 2. Environmental Effects (Rain, Stars, Clouds)

Environmental particles are spawned dynamically based on the player's position and world state to create the illusion of a massive world with minimal overhead.

### Spawning Logic
In [game.c](file:///home/pes/dev/agent/zvirus-conv-raylib/src/game.c#L53-L90):
- **Rain (Template 9):** Likelihood is calculated based on `gAreaInfected` (world health). It is spawned "ahead" of the player along their velocity vector using `G_RAIN_LEAD` to ensure the player always flies into "fresh" rain.
- **Stars/Dust (Template 16):** Spawned relative to the player when altitude $> 25.0$ (`G_ENV_THRESHOLD`).
- **Cloud Layer (Template 17):** Spawned when altitude exceeds `MAX_HEIGHT` (150).

## 3. Ground Interaction Logic

Every particle update ([particle.c:L135-L164](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#L135-164)) checks for ground collisions using a tiered system.

### Basic Collision (Rain & Splashes)
1. **Height Sampling:** Calls `TerrainGetHeight(p->x, p->z, 1)` to get the height including world objects.
2. **Impact Detection:** If $p->y < groundHeight$:
   - The particle reflects its vertical velocity (`p->vy *= -0.5f`).
   - If the template has a `splash` count, it spawns new splash particles at the impact point ([particle.c:L252-L260](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#L252-260)).

### Infection Spread
When an infection-related particle (Template 10 or 11) hits the ground, it calls `TerrainMapAdd(0, ...)` in [terrain.c:L583](file:///home/pes/dev/agent/zvirus-conv-raylib/src/terrain.c#L583).
- **Effect:** Sets `landInfected = 1` for a localized area.
- **Mutation:** If a "Healthy" object (Status 0) is on that land, it is changed to "Infected" (Status 1).
- **Minimap Dirty Tracking:** To save performance, the change is queued in `gMapChange` rather than re-rendering the whole minimap texture immediately.

## 4. Collision Precision (Projectiles vs. Decoration)

To maximize performance, the engine uses different collision levels:
- **Decoration (Rain/Smoke):** Uses simple "Block Height" comparison (Terrain Height + Pre-calculated Mesh Height).
- **Projectiles (Bullets):** For specific templates (ID 1, 12, 18), the engine performs **Ray-to-Mesh** collision against 3D models (like Trees/Buildings) using `GetRayCollisionMesh` ([particle.c:L169-L210](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#L169-210)).

## 5. Visual Rendering Tricks

- **Billboarding:** Particles are rendered as 2D quads facing the camera ([particle.c:L313](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#L313)), making them extremely cheap to draw by the thousands.
- **Vertex Color Animation:** Changes to "Infected" terrain are visualized by swapping the vertex color of the scrolling terrain mesh during the mesh update loop ([terrain.c:L153-L157](file:///home/pes/dev/agent/zvirus-conv-raylib/src/terrain.c#L153-157).
- **Shadow Pass:** Particles are rendered using a manual `rlgl` billboard implementation during the shadow map pass ([particle.c:L316-L343](file:///home/pes/dev/agent/zvirus-conv-raylib/src/particle.c#316-343)). This ensures that the Model-View-Projection (MVP) matrix is explicitly synchronized for every particle, overcoming limitations in high-level utility functions when used with custom shadow shaders. This transition preserves the 6x reduction in triangle count (2 vs 12) while guaranteeing robust depth capture.
