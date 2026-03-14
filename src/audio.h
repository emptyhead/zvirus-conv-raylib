#pragma once

#include "raylib.h"
#include "world.h"

// Sound handles
extern Sound gSoundWind;
extern Sound gSoundThunder;
extern Sound gSoundShoot;
extern Sound gSoundSmall;
extern Sound gSoundExplode;
extern Sound gSoundSplat;
extern Sound gSoundLaunch;
extern Sound gSoundMenu;
extern Sound gSoundAllie;
extern Sound gSoundBabble;
extern Sound gSoundFirework;
extern Sound gSoundFanfare;
extern Sound gSoundSplash;

// Channels (Raylib doesn't have explicit channels like Blitz, 
// but we can manage multi-sample playback)

void AudioInit(void);
void AudioUpdate(void);
void AudioPlay(Sound sound, float volume, float pan);
void AudioPauseAmbient(bool pause);
