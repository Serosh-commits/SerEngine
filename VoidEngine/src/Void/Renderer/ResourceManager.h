#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Texture.h"
#include "Shader.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace Void {

    class ResourceManager {
    public:
        static std::shared_ptr<Texture2D> LoadTexture(const std::string& path);
        static std::shared_ptr<Shader> LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        static std::shared_ptr<Shader> LoadShaderFromSrc(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);

        static std::shared_ptr<Texture2D> GetTexture(const std::string& path);
        static std::shared_ptr<Shader> GetShader(const std::string& name);

        static void Shutdown();

    private:
        static std::unordered_map<std::string, std::shared_ptr<Texture2D>> s_Textures;
        static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
    };

}

#endif
