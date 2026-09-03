#include "miniaudio.h"
#include <cstdio>

enum SoundType {
    SOUND_START,
    SOUND_END
};

class Sound 
{
private:
    ma_engine SoundEngine;
    ma_sound  StartSound;
    ma_sound  EndSound;

    bool LoadSound(const char* filename, ma_sound* sound)
    {
        ma_result result = ma_sound_init_from_file(&SoundEngine, filename, 0, NULL, NULL, sound);
        if (result != MA_SUCCESS) 
        {
            printf("Failed to load sound: %s\n", filename);
            return false;
        }
        return true;
    }

public:
    bool Init()
    {
        ma_result result;
        result = ma_engine_init(NULL, &SoundEngine);
        if (result != MA_SUCCESS) 
        {
            printf("Failed to initialize audio engine\n");
            return false;
        }

        return true;
    }

    bool LoadSounds()
    {
        if (!LoadSound("data/sound/start.wav", &StartSound)) return false;
        if (!LoadSound("data/sound/end.wav", &EndSound)) return false;
        return true;
    }

    void Done()
    {
        ma_sound_uninit(&StartSound);
        ma_sound_uninit(&EndSound);
        ma_engine_uninit(&SoundEngine);
    }

   

    bool PlaySound(SoundType sound)
    {
        ma_sound* maSound = nullptr;
        switch (sound) {
            case SOUND_START: maSound = &StartSound; break;
            case SOUND_END: maSound = &EndSound; break;
            default: return false;
        }

        ma_result result = ma_sound_start(maSound);
        if (result != MA_SUCCESS) 
        {
            printf("Failed to play sound\n");
            return false;
        }
        return true;
    }
};