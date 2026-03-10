#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace Void {

    struct ParticleProps {
        glm::vec2 Position = { 0.0f, 0.0f };
        glm::vec2 Velocity = { 0.0f, 0.0f };
        glm::vec2 VelocityVariation = { 1.0f, 1.0f };
        glm::vec4 ColorBegin = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 ColorEnd = { 1.0f, 1.0f, 1.0f, 0.0f };
        float SizeBegin = 0.5f;
        float SizeEnd = 0.0f;
        float SizeVariation = 0.3f;
        float LifeTime = 1.0f;
    };

    class ParticleSystem {
    public:
        ParticleSystem(uint32_t maxParticles = 1000);

        void Emit(const ParticleProps& props);
        void OnUpdate(float ts);
        void OnRender();

    private:
        struct Particle {
            glm::vec2 Position;
            glm::vec2 Velocity;
            glm::vec4 ColorBegin, ColorEnd;
            float Rotation = 0.0f;
            float SizeBegin, SizeEnd;
            float LifeTime = 1.0f;
            float LifeRemaining = 0.0f;
            bool Active = false;
        };

        std::vector<Particle> m_ParticlePool;
        uint32_t m_PoolIndex = 0;
    };

}

#endif
