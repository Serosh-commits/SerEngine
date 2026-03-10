#ifndef RENDERER2D_H
#define RENDERER2D_H

#include "OrthographicCamera.h"
#include "Camera.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <memory>

namespace Void {

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();
        static void Flush();

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        struct Statistics {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
        };

        static void ResetStats();
        static Statistics GetStats();

        static void SetAmbientLight(float intensity, const glm::vec3& color = {1.0f, 1.0f, 1.0f});
        static void SubmitLight(const glm::vec3& position, const glm::vec3& color, float intensity, float radius);
        static void SubmitSpotlight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float intensity, float cutoff, float outerCutoff);
        static void SubmitVisibilityMap(const glm::vec2& gridOrigin, float cellSize, int gridSize, const float* data);

        static std::shared_ptr<class Shader> GetShader();

    private:
    };

}

#endif
