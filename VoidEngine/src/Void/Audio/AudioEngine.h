#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Void {

    class AudioEngine {
    public:
        static void Init();
        static void Shutdown();

        static void PlaySound(const std::string& path);
        static void PlayMusic(const std::string& path, bool loop = true);
        static void PlaySpatialSound(const std::string& path, const glm::vec3& position, float volume = 1.0f, float pitch = 1.0f);

        static void SetListenerPosition(const glm::vec3& position);
    };

}

#endif
