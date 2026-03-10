#ifndef CAMERA_EFFECTS_H
#define CAMERA_EFFECTS_H

#include <glm/glm.hpp>
#include <random>

namespace Void {

    class CameraShake {
    public:
        void Trigger(float intensity, float duration, float frequency = 30.0f) {
            m_Intensity = intensity;
            m_Duration = duration;
            m_Remaining = duration;
            m_Frequency = frequency;
        }

        void Update(float ts) {
            if (m_Remaining <= 0.0f) {
                m_Offset = { 0.0f, 0.0f };
                return;
            }

            m_Remaining -= ts;
            float decay = m_Remaining / m_Duration;
            float strength = m_Intensity * decay * decay;

            static std::mt19937 rng(42);
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

            m_Timer += ts;
            float step = 1.0f / m_Frequency;
            if (m_Timer >= step) {
                m_Timer -= step;
                m_TargetOffset = { dist(rng) * strength, dist(rng) * strength };
            }

            m_Offset = glm::mix(m_Offset, m_TargetOffset, ts * 20.0f);
        }

        glm::vec2 GetOffset() const { return m_Offset; }
        bool IsActive() const { return m_Remaining > 0.0f; }

    private:
        float m_Intensity = 0.0f;
        float m_Duration = 0.0f;
        float m_Remaining = 0.0f;
        float m_Frequency = 30.0f;
        float m_Timer = 0.0f;
        glm::vec2 m_Offset = { 0.0f, 0.0f };
        glm::vec2 m_TargetOffset = { 0.0f, 0.0f };
    };

}

#endif
