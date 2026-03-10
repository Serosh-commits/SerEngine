#include "Texture.h"
#include "SerGame/Core/Log.h"
#include <epoxy/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SerGame {

    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D(const std::string& path) : m_Path(path) {
            int width, height, channels;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

            if (!data) {
                SER_CORE_ERROR("Failed to load image: {0}", path);
                return;
            }

            m_Width = width;
            m_Height = height;

            GLenum internalFormat = 0, dataFormat = 0;
            if (channels == 4) {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            } else if (channels == 3) {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            if (!internalFormat || !dataFormat) {
                SER_CORE_ERROR("Unsupported texture format! ({0} channels)", channels);
                stbi_image_free(data);
                return;
            }

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            
            stbi_image_free(data);
        }

        virtual ~OpenGLTexture2D() {
            glDeleteTextures(1, &m_RendererID);
        }

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual void Bind(uint32_t slot) const override {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
        }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
    };

    std::shared_ptr<Texture> Texture::Create(const std::string& path) {
        return std::make_shared<OpenGLTexture2D>(path);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path) {
        return std::make_shared<OpenGLTexture2D>(path);
    }

}
