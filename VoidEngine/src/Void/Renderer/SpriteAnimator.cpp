#include "SpriteAnimator.h"
#include "Texture.h"

namespace Void {

    void SpriteAnimator::AddAnimation(const std::string& name, const std::shared_ptr<Texture2D>& spriteSheet,
                                       int frameWidth, int frameHeight, int startFrame, int frameCount,
                                       float frameDuration, bool looping) {
        SpriteAnimation anim;
        anim.Name = name;
        anim.Looping = looping;

        int texW = spriteSheet->GetWidth();
        int texH = spriteSheet->GetHeight();
        m_SheetCols = texW / frameWidth;
        m_SheetRows = texH / frameHeight;

        for (int i = 0; i < frameCount; i++) {
            int idx = startFrame + i;
            int col = idx % m_SheetCols;
            int row = idx / m_SheetCols;

            AnimationFrame frame;
            frame.u0 = (float)col * frameWidth / (float)texW;
            frame.v0 = (float)row * frameHeight / (float)texH;
            frame.u1 = (float)(col + 1) * frameWidth / (float)texW;
            frame.v1 = (float)(row + 1) * frameHeight / (float)texH;
            frame.Duration = frameDuration;

            anim.Frames.push_back(frame);
        }

        m_Animations[name] = anim;
    }

    void SpriteAnimator::Play(const std::string& name) {
        if (m_CurrentAnimation == name && m_Playing) return;
        m_CurrentAnimation = name;
        m_CurrentFrame = 0;
        m_Timer = 0.0f;
        m_Playing = true;
    }

    void SpriteAnimator::Stop() {
        m_Playing = false;
    }

    void SpriteAnimator::Update(float ts) {
        if (!m_Playing || m_CurrentAnimation.empty()) return;

        auto it = m_Animations.find(m_CurrentAnimation);
        if (it == m_Animations.end()) return;

        auto& anim = it->second;
        m_Timer += ts;

        if (m_Timer >= anim.Frames[m_CurrentFrame].Duration) {
            m_Timer = 0.0f;
            m_CurrentFrame++;
            if (m_CurrentFrame >= (int)anim.Frames.size()) {
                if (anim.Looping)
                    m_CurrentFrame = 0;
                else {
                    m_CurrentFrame = (int)anim.Frames.size() - 1;
                    m_Playing = false;
                }
            }
        }
    }

    void SpriteAnimator::GetCurrentUVs(float& u0, float& v0, float& u1, float& v1) const {
        auto it = m_Animations.find(m_CurrentAnimation);
        if (it == m_Animations.end()) {
            u0 = 0; v0 = 0; u1 = 1; v1 = 1;
            return;
        }
        auto& frame = it->second.Frames[m_CurrentFrame];
        u0 = frame.u0; v0 = frame.v0;
        u1 = frame.u1; v1 = frame.v1;
    }

}
