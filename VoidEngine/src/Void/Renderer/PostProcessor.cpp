#include "PostProcessor.h"
#include <epoxy/gl.h>

namespace Void {

    PostProcessor::PostProcessor() {

        m_FullscreenQuadVAO.reset(VertexArray::Create());

        float vertices[] = {

            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        std::shared_ptr<VertexBuffer> vbo;
        vbo.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
        vbo->SetLayout({
            { ShaderDataType::Float2, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        m_FullscreenQuadVAO->AddVertexBuffer(vbo);

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        std::shared_ptr<IndexBuffer> ibo;
        ibo.reset(IndexBuffer::Create(indices, 6));
        m_FullscreenQuadVAO->SetIndexBuffer(ibo);
    }

    PostProcessor::~PostProcessor() {
    }

    void PostProcessor::Render(const std::shared_ptr<Framebuffer>& framebuffer, const std::shared_ptr<Shader>& postProcessShader) {

        postProcessShader->Bind();
        postProcessShader->UploadUniformInt("u_Texture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer->GetTextureID());

        m_FullscreenQuadVAO->Bind();

        glDisable(GL_DEPTH_TEST); 
        glDrawElements(GL_TRIANGLES, m_FullscreenQuadVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
        glEnable(GL_DEPTH_TEST);
    }

}
