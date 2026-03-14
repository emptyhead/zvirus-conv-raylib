# Source.bb to C/RayLib Mapping Analysis

This document provides a detailed mapping between the original BlitzBasic ([Source.bb](file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb)) and the ported C project.

## Data Structures (Types)

| Blitz Type (Source.bb) | Lines | C Struct (world.h) | Lines |
| :--- | :--- | :--- | :--- |
| `SoundGroupType` | 78-86 | `SoundGroup` | 7-16 |
| `MenuType` | 88-103 | `Menu` | 18-34 |
| `ScoreTagType` | 105-109 | [ScoreTag](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c#36-50) | 36-40 |
| `ParticleTemplateType` | 111-123 | `ParticleTemplate` | 42-50 |
| `ParticleType` | 125-135 | [Particle](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c#82-104) | 52-61 |
| `GroundObjectType` | 137-140 | `GroundObject` | 63-66 |
| `FlyingObjectType` | 142-164 | `FlyingObject` | 68-90 |
| `GridType` | 166-173 | [Grid](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#16-75) | 92-99 |
| `TerrainType` | 175-190 | [Terrain](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#160-301) | 101-118 |
| `HudType` | 192-207 | [Hud](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#11-16) | 120-135 |
| `PlayerType` | 209-232 | [Ship](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#243-376) | 137-170 |

## Global Variables & Constants

| Blitz Global/Const | Line | C External (world.h) | Line | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `FPS = 30` | 30 | `FPS = 60` | 6 | (constants.h) 60Hz parity applied |
| `Gravity#` | 19 | `gGravity` | 177 | |
| `FadeStatus#` | 20 | `gFadeStatus` | 195 | |
| `Scores` | 24 | `gScore` | 194 | |
| `Enemies` | 61 | `gEnemies` | 193 | |
| `AreaTotal`, `AreaInfected` | 61 | `gAreaTotal`, `gAreaInfected` | 196 | |
| `Grid.GridType` | 234 | `gGrid` | 178 | |
| `HUD.HudType` | 235 | `gHud` | 179 | |
| [Terrain()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#160-301) | 237 | `gTerrain[][]` | 181 | |
| `GroundObject()` | 238 | `gGroundObjects[]` | 182 | |
| `FlyingObject()` | 239 | `gFlyingObjects[]` | 183 | |
| `Player()` | 240 | `gShips[]` | 184 | |
| [Particle()](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c#82-104) | 241 | `gParticles[]` | 185 | |
| [ScoreTag()](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c#36-50) | 243 | `gScoreTags[]` | 187 | |
| `MENU()` | 244 | `gMenus[]` | 188 | |
| `DistanceLUP()` | 246 | `gDistanceLUP[][]` | 190 | |
| `MaxPlayers` | 31 | `gMaxShips` | 191 | |

### Action Globals (Inputs/Intents)
These are used for AI logic and player control.

| Blitz Global | Line | C Equivalent | File | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `JX#`, `JZ#` | 22 | `gJX`, `gJZ` | `ship.c:471` | Mirrored as globals or [Ship](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#243-376) fields (`jxIntent`) |
| `Thrust#` | 22 | `gThrust` | `ship.c:472` | Mirrored as `thrustIntent` in [Ship](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#243-376) |
| `Shoot` | 22 | `gShoot` | `ship.c:472` | |
| [Launch](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#377-419) | 22 | `gLaunch` | `ship.c:472` | |
| `Infect`, `Bomb` | 22 | `gInfect`, `gBomb` | `ship.c:473` | |

## Core Functions

| Blitz Function | Line | C Function | Line | File |
| :--- | :--- | :--- | :--- | :--- |
| `WAVEstart()` | 287 | [WaveStart()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#97-188) | 97 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `WAVEend()` | 424 | [WaveEnd()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#189-214) | 189 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `COLLISIONground()` | 446 | [TerrainCollisionGround()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#446-470) | 446 | [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) |
| `COLLISIONbullet()` | 480 | [CollisionBullet()](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c#15-46) | 15 | [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c) |
| `COLLISIONplayer()` | 511 | [CollisionShip()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#420-467) | 423 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERreset()` | 561 | [ShipReset()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#91-137) | 94 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERupdate()` | 624 | [ShipUpdateAll()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#475-735) | 480 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERposition()` | 928 | [ShipPosition()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#138-182) | 141 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) (static) |
| `PLAYERai()` | 942 | [ShipAI()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#243-376) | 246 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERlaunch()` | 1105 | [ShipLaunch()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#377-419) | 380 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERchase()` | 1146 | [ShipChase()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#202-242) | 205 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `PLAYERfindtarget()`| 1168 | [ShipFindTarget()](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c#183-201) | 186 | [ship.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/ship.c) |
| `SCOREtagadd()` | 1224 | [ScoreTagAdd()](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c#36-50) | 36 | [score_tag.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c) |
| `SCOREtagupdate()` | 1245 | [ScoreTagUpdateAll()](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c#51-67) | 51 | [score_tag.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/score_tag.c) |
| `GAMEinit()` | 1267 | [GameInit()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#12-21) | 12 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `GAMEstart()` | 1399 | [GameStart()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#22-33) | 22 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `GAMEupdate()` | 1436 | [GameUpdate()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#34-91) | 34 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `GAMEend()` | 1560 | [GameEnd()](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c#92-96) | 92 | [game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/game.c) |
| `MAPadd()` | 1576 | [TerrainMapAdd()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#410-445) | 414 | [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) |
| `MAPupdate()` | 1628 | [HudUpdateMap()](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#21-50) | 21 | [hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c) |
| `TERRAINupdate()` | 1646 | [TerrainUpdateGrid()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#77-158) | 77 | [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) |
| `TERRAINload()` | 1807 | [TerrainLoad()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#160-301) | 161 | [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) |
| `TERRAINgetheight#()`| 1926 | [TerrainGetHeight()](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c#303-341) | 304 | [terrain.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c) |
| `CAMERAupdate()` | 1954 | [CameraGameUpdate()](file:///home/pes/dev/C/zvirus-conv-raylib/src/camera_game.c#25-81) | 29 | [camera_game.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/camera_game.c) |
| `PARTICLEupdate()` | 2271 | [ParticleUpdateAll()](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c#105-156) | 105 | [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c) |
| `PARTICLEnew()` | 2358 | [ParticleNew()](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c#82-104) | 82 | [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c) |
| `HUDinit()` | 2508 | [HudInit()](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#11-16) | 11 | [hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c) |
| `HUDupdate()` | 2552 | [HudDraw()](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#52-118) | 52 | [hud.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c) (also [HudUpdate](file:///home/pes/dev/C/zvirus-conv-raylib/src/hud.c#17-20)) |
| `MENUinit()` | 2607 | `MenuInit()` | | [menu.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/menu.c) (needs verification) |
| `MENUshow()` | 2655 | `MenuDraw()` | 201 | [main.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c) (delegates) |
| `MENUupdate()` | 2710 | `MenuUpdate()` | | [menu.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/menu.c) (needs verification) |

> [!NOTE]
> The C implementation uses a 60Hz physics clock (`FPS = 60`), whereas the original Blitz code ran at 30Hz. Many movement and timing constants in [constants.h](file:///home/pes/dev/C/zvirus-conv-raylib/src/constants.h) have been scaled by 0.5 or similar factors to maintain gameplay parity.
