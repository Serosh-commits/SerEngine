#include "Buffer.h"
#include <epoxy/gl.h>

namespace SerGame {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size) {
            glGenBuffers(1, &m_RendererID);
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
            glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        }

        virtual ~OpenGLVertexBuffer() {
            glDeleteBuffers(1, &m_RendererID);
        }

        virtual void Bind() const override {
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        }

        virtual void Unbind() const override {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

    private:
        uint32_t m_RendererID;
        BufferLayout m_Layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count) {
            glGenBuffers(1, &m_RendererID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        }

        virtual ~OpenGLIndexBuffer() {
            glDeleteBuffers(1, &m_RendererID);
        }

        virtual void Bind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        }

        virtual void Unbind() const override {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        virtual uint32_t GetCount() const override { return m_Count; }

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };

    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
        return new OpenGLVertexBuffer(vertices, size);
    }

    IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        return new OpenGLIndexBuffer(indices, count);
    }

}
