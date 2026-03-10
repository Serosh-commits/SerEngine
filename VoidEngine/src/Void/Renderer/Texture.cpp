#include "Texture.h"
#include "Void/Core/Log.h"
#include <epoxy/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Void {

    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(uint32_t width, uint32_t height)
            : m_Width(width), m_Height(height) {

            m_InternalFormat = GL_RGBA8;
            m_DataFormat = GL_RGBA;

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        OpenGLTexture2D(const std::string& path) : m_Path(path) {
            int width, height, channels;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

            if (!data) {
                VOID_CORE_ERROR("Failed to load image: {0}", path);
                m_Width = m_Height = 0;
                return;
            }

            m_Width = width;
            m_Height = height;

            m_InternalFormat = GL_RGBA8;
            m_DataFormat = GL_RGBA;

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }

        virtual ~OpenGLTexture2D() {
            glDeleteTextures(1, &m_RendererID);
        }

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetRendererID() const override { return m_RendererID; }

        virtual void SetData(void* data, uint32_t size) override {
            uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
            if (size != m_Width * m_Height * bpp) {
                VOID_CORE_ERROR("Data must be entire texture!");
            }
            glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
        }

        virtual void Bind(uint32_t slot) const override {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
        }

        virtual bool operator==(const Texture& other) const override {
            return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
        }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;
    };

    std::shared_ptr<Texture> Texture::Create(const std::string& path) {
        return std::make_shared<OpenGLTexture2D>(path);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
        return std::make_shared<OpenGLTexture2D>(width, height);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path) {
        return std::make_shared<OpenGLTexture2D>(path);
    }

}
