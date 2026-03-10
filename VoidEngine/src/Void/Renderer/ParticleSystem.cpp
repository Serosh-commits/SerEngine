#include "ParticleSystem.h"
#include "Renderer2D.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace Void {

    static std::mt19937 s_RNG;
    static std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);

    static float RandomFloat() { return s_Distribution(s_RNG); }

    ParticleSystem::ParticleSystem(uint32_t maxParticles) {
        m_ParticlePool.resize(maxParticles);
    }

    void ParticleSystem::Emit(const ParticleProps& props) {
        Particle& p = m_ParticlePool[m_PoolIndex];
        p.Active = true;
        p.Position = props.Position;
        p.Rotation = RandomFloat() * 2.0f * glm::pi<float>();

        p.Velocity = props.Velocity;
        p.Velocity.x += props.VelocityVariation.x * (RandomFloat() - 0.5f);
        p.Velocity.y += props.VelocityVariation.y * (RandomFloat() - 0.5f);

        p.ColorBegin = props.ColorBegin;
        p.ColorEnd = props.ColorEnd;
        p.LifeTime = props.LifeTime;
        p.LifeRemaining = props.LifeTime;
        p.SizeBegin = props.SizeBegin + props.SizeVariation * (RandomFloat() - 0.5f);
        p.SizeEnd = props.SizeEnd;

        m_PoolIndex = (m_PoolIndex + 1) % m_ParticlePool.size();
    }

    void ParticleSystem::OnUpdate(float ts) {
        for (auto& p : m_ParticlePool) {
            if (!p.Active) continue;

            p.LifeRemaining -= ts;
            if (p.LifeRemaining <= 0.0f) {
                p.Active = false;
                continue;
            }

            p.Position += p.Velocity * ts;
            p.Rotation += 0.2f * ts;
        }
    }

    void ParticleSystem::OnRender() {
        for (auto& p : m_ParticlePool) {
            if (!p.Active) continue;

            float lifeRatio = p.LifeRemaining / p.LifeTime;
            glm::vec4 color = glm::mix(p.ColorEnd, p.ColorBegin, lifeRatio);
            color.a *= lifeRatio;

            float size = glm::mix(p.SizeEnd, p.SizeBegin, lifeRatio);

            glm::mat4 transform = glm::translate(glm::mat4(1.0f), { p.Position.x, p.Position.y, 0.1f })
                * glm::rotate(glm::mat4(1.0f), p.Rotation, { 0.0f, 0.0f, 1.0f })
                * glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

            Renderer2D::DrawQuad(transform, color);
        }
    }

}
