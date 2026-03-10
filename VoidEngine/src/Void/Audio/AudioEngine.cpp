#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioEngine.h"
#include "Void/Core/Log.h"

namespace Void {

    struct AudioData {
        ma_engine Engine;
        bool Initialized = false;
    };

    static AudioData s_Audio;

    void AudioEngine::Init() {
        ma_result result = ma_engine_init(NULL, &s_Audio.Engine);
        if (result != MA_SUCCESS) {
            VOID_CORE_ERROR("Failed to initialize audio engine");
            return;
        }
        s_Audio.Initialized = true;
        VOID_CORE_INFO("Audio Engine Initialized.");
    }

    void AudioEngine::Shutdown() {
        if (s_Audio.Initialized)
            ma_engine_uninit(&s_Audio.Engine);
    }

    void AudioEngine::PlaySound(const std::string& path) {
        if (!s_Audio.Initialized) return;
        ma_engine_play_sound(&s_Audio.Engine, path.c_str(), NULL);
    }

    void AudioEngine::PlayMusic(const std::string& path, bool loop) {
        if (!s_Audio.Initialized) return;

        ma_sound* sound = new ma_sound();
        ma_result result = ma_sound_init_from_file(&s_Audio.Engine, path.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, sound);
        if (result == MA_SUCCESS) {
            ma_sound_set_looping(sound, loop);
            ma_sound_start(sound);
        } else {
            VOID_CORE_ERROR("Failed to load music: {0}", path);
            delete sound;
        }
    }

    void AudioEngine::PlaySpatialSound(const std::string& path, const glm::vec3& position, float volume, float pitch) {
        if (!s_Audio.Initialized) return;

        ma_sound* sound = new ma_sound();
        ma_result result = ma_sound_init_from_file(&s_Audio.Engine, path.c_str(), 0, NULL, NULL, sound);
        if (result == MA_SUCCESS) {
            ma_sound_set_position(sound, position.x, position.y, position.z);
            ma_sound_set_volume(sound, volume);
            ma_sound_set_pitch(sound, pitch);
            ma_sound_start(sound);
        } else {
            delete sound;
        }
    }

    void AudioEngine::SetListenerPosition(const glm::vec3& position) {
        if (!s_Audio.Initialized) return;
        ma_engine_listener_set_position(&s_Audio.Engine, 0, position.x, position.y, position.z);
    }

}
