#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include "Void/Renderer/Framebuffer.h"
#include "Void/Renderer/Shader.h"
#include "Void/Renderer/VertexArray.h"
#include <memory>

namespace Void {

    class PostProcessor {
    public:
        PostProcessor();
        ~PostProcessor();

        void Render(const std::shared_ptr<Framebuffer>& framebuffer, const std::shared_ptr<Shader>& postProcessShader);

    private:
        std::shared_ptr<VertexArray> m_FullscreenQuadVAO;
    };

}

#endif
