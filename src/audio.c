#include <stdio.h>
#include "audio.h"
#include "world.h"

#define VOICE_POOL_SIZE 4

// Re-defining SoundGroup to hold Raylib Sound objects
typedef struct AudioGroup {
    Sound sound;
    Sound aliases[VOICE_POOL_SIZE];
    int nextAlias;
    bool playing;
} AudioGroup;

static AudioGroup gGroups[SOUND_GROUPS + 1];

// Voice pool for global spot effects
static Sound gSoundShootPool[VOICE_POOL_SIZE];
static int gNextShoot = 0;
static Sound gSoundExplodePool[VOICE_POOL_SIZE];
static int gNextExplode = 0;
static Sound gSoundMenuPool[VOICE_POOL_SIZE];
static int gNextMenu = 0;

Sound gSoundWind;
Sound gSoundThunder;
Sound gSoundShoot;
Sound gSoundSmall;
Sound gSoundExplode;
Sound gSoundSplat;
Sound gSoundLaunch;
Sound gSoundMenu;
Sound gSoundAllie;
Sound gSoundBabble;
Sound gSoundFirework;
Sound gSoundFanfare;
Sound gSoundSplash;

static const char *DATA_PATH = "SoundData2/";

void AudioInit(void) {
    if (!IsAudioDeviceReady()) return;

    gSoundWind     = LoadSound(TextFormat("%sWind.wav", DATA_PATH));
    gSoundThunder  = LoadSound(TextFormat("%sThunder.wav", DATA_PATH));
    gSoundShoot    = LoadSound(TextFormat("%sShoot.wav", DATA_PATH));
    gSoundSmall    = LoadSound(TextFormat("%sSmall.wav", DATA_PATH));
    gSoundExplode  = LoadSound(TextFormat("%sExplosion.wav", DATA_PATH));
    gSoundSplat    = LoadSound(TextFormat("%sSplat.wav", DATA_PATH));
    gSoundLaunch   = LoadSound(TextFormat("%sLaunch.wav", DATA_PATH));
    gSoundMenu     = LoadSound(TextFormat("%sbutton.wav", DATA_PATH));
    gSoundAllie    = LoadSound(TextFormat("%sdrumroll.wav", DATA_PATH));
    gSoundBabble   = LoadSound(TextFormat("%sBabble.wav", DATA_PATH));
    gSoundFirework = LoadSound(TextFormat("%sFirework.wav", DATA_PATH));
    gSoundFanfare  = LoadSound(TextFormat("%sFanfare.wav", DATA_PATH));
    gSoundSplash   = LoadSound(TextFormat("%sSplash.wav", DATA_PATH));

    // Initialize global pools
    for (int i = 0; i < VOICE_POOL_SIZE; i++) {
        gSoundShootPool[i] = LoadSoundAlias(gSoundShoot);
        gSoundExplodePool[i] = LoadSoundAlias(gSoundExplode);
        gSoundMenuPool[i] = LoadSoundAlias(gSoundMenu);
    }

    // Sound Groups for 3D sounds
    const char* sgNames[] = {
        "thrustloop", "Seeder", "Pest", "Attractor", "Missile", 
        "Generator", "Repulsor", "Pest", "Monster", "Thrust2"
    };

    for (int i = 0; i <= SOUND_GROUPS; i++) {
        char path[256];
        if (i == 9) snprintf(path, 256, "%s%s.WAV", DATA_PATH, sgNames[i]);
        else snprintf(path, 256, "%s%s.wav", DATA_PATH, sgNames[i]);
        
        gGroups[i].sound = LoadSound(path);
        gGroups[i].playing = false;
        gGroups[i].nextAlias = 0;
        for (int v = 0; v < VOICE_POOL_SIZE; v++) {
            gGroups[i].aliases[v] = LoadSoundAlias(gGroups[i].sound);
        }
        gSoundGroups[i].sound = i;
    }
}

static bool gAmbientActive = false;

void AudioUpdate(void) {
    if (!IsAudioDeviceReady()) return;

    if (gAmbientActive && !IsSoundPlaying(gSoundWind)) {
        SetSoundVolume(gSoundWind, 0.25f);
        PlaySound(gSoundWind);
    }

    // Update looping 3D sounds
    for (int i = 0; i <= SOUND_GROUPS; i++) {
        SoundGroup *sg = &gSoundGroups[i];
        
        float vol = sg->volume;
        if (vol > 0.001f) {
            // For looping group sounds, we use the primary instance
            if (!IsSoundPlaying(gGroups[i].sound)) {
                PlaySound(gGroups[i].sound);
            }
            SetSoundVolume(gGroups[i].sound, vol);
            SetSoundPan(gGroups[i].sound, 1.0f - sg->pan); // Raylib: 0.0=right,1.0=left (inverted from our convention)
        } else {
            if (IsSoundPlaying(gGroups[i].sound)) {
                StopSound(gGroups[i].sound);
            }
        }
        
        sg->volume = 0;
        sg->pan    = 0.5f; // Reset to centre
        sg->distance = 10000.0f;
    }
}

void AudioPlay(Sound sound, float volume, float pan) {
    Sound voice = sound;
    
    // Check if this is one of our pooled sounds (comparing buffers)
    if (sound.stream.buffer == gSoundShoot.stream.buffer) {
        voice = gSoundShootPool[gNextShoot];
        gNextShoot = (gNextShoot + 1) % VOICE_POOL_SIZE;
    } else if (sound.stream.buffer == gSoundExplode.stream.buffer) {
        voice = gSoundExplodePool[gNextExplode];
        gNextExplode = (gNextExplode + 1) % VOICE_POOL_SIZE;
    } else if (sound.stream.buffer == gSoundMenu.stream.buffer) {
        voice = gSoundMenuPool[gNextMenu];
        gNextMenu = (gNextMenu + 1) % VOICE_POOL_SIZE;
    }

    SetSoundVolume(voice, volume);
    SetSoundPan(voice, 1.0f - pan); // Raylib: 0.0=right,1.0=left (inverted from our convention)
    PlaySound(voice);
}

void AudioPauseAmbient(bool pause) {
    gAmbientActive = !pause;
    if (pause) {
        StopSound(gSoundWind);
    } else {
        if (!IsSoundPlaying(gSoundWind)) {
             SetSoundVolume(gSoundWind, 0.25f);
             PlaySound(gSoundWind);
        }
    }
}
