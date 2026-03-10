#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdint>

namespace Void {

    class Framebuffer {
    public:
        Framebuffer(uint32_t width, uint32_t height);
        ~Framebuffer();

        void Bind();
        void Unbind();

        uint32_t GetTextureID() const { return m_ColorAttachment; }

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_ColorAttachment = 0;
        uint32_t m_DepthAttachment = 0;
    };

}

#endif
