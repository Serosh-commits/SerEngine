#ifndef SPRITE_ANIMATOR_H
#define SPRITE_ANIMATOR_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace Void {

    class Texture2D;

    struct AnimationFrame {
        float u0, v0, u1, v1;
        float Duration;
    };

    struct SpriteAnimation {
        std::string Name;
        std::vector<AnimationFrame> Frames;
        bool Looping = true;
    };

    class SpriteAnimator {
    public:
        SpriteAnimator() = default;

        void AddAnimation(const std::string& name, const std::shared_ptr<Texture2D>& spriteSheet,
                          int frameWidth, int frameHeight, int startFrame, int frameCount,
                          float frameDuration, bool looping = true);

        void Play(const std::string& name);
        void Stop();
        void Update(float ts);

        bool IsPlaying() const { return m_Playing; }
        const std::string& GetCurrentAnimation() const { return m_CurrentAnimation; }
        int GetCurrentFrame() const { return m_CurrentFrame; }
        
        void GetCurrentUVs(float& u0, float& v0, float& u1, float& v1) const;

    private:
        std::unordered_map<std::string, SpriteAnimation> m_Animations;
        std::string m_CurrentAnimation;
        int m_CurrentFrame = 0;
        float m_Timer = 0.0f;
        bool m_Playing = false;
        
        int m_SheetCols = 1;
        int m_SheetRows = 1;
    };

}

#endif
