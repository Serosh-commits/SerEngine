#include "ResourceManager.h"
#include <fstream>
#include <sstream>

namespace Void {

    std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::s_Textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;

    std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const std::string& path) {
        if (s_Textures.find(path) != s_Textures.end())
            return s_Textures[path];

        auto texture = Texture2D::Create(path);
        s_Textures[path] = texture;
        return texture;
    }

    std::shared_ptr<Shader> ResourceManager::LoadShaderFromSrc(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
        if (s_Shaders.find(name) != s_Shaders.end())
            return s_Shaders[name];

        auto shader = std::make_shared<Shader>(vertexSrc, fragmentSrc);
        s_Shaders[name] = shader;
        return shader;
    }

    std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& path) {
        return s_Textures.count(path) ? s_Textures[path] : nullptr;
    }

    std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name) {
        return s_Shaders.count(name) ? s_Shaders[name] : nullptr;
    }

    void ResourceManager::Shutdown() {
        s_Textures.clear();
        s_Shaders.clear();
    }

}
