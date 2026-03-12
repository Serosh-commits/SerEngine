#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Void/Core/Log.h"
#include <epoxy/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace Void {

    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct Spotlight {
        glm::vec3 Position;
        glm::vec3 Direction;
        glm::vec3 Color;
        float Intensity;
        float CutOff;
        float OuterCutOff;
    };

    struct Renderer2DData {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> TextureShader;
        std::shared_ptr<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        uint32_t LightCount = 0;
        Spotlight Flashlight;
        bool FlashlightEnabled = false;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1;

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        s_Data.QuadVertexArray.reset(VertexArray::Create());

        s_Data.QuadVertexBuffer.reset(VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex)));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_TilingFactor" }
        });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;
            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;
            offset += 4;
        }

        std::shared_ptr<IndexBuffer> quadIB;
        quadIB.reset(IndexBuffer::Create(quadIndices, s_Data.MaxIndices));
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;
            layout(location = 2) in vec2 a_TexCoord;
            layout(location = 3) in float a_TexIndex;
            layout(location = 4) in float a_TilingFactor;

            uniform mat4 u_ViewProjection;

            out vec4 v_Color;
            out vec2 v_TexCoord;
            out float v_TexIndex;
            out float v_TilingFactor;
            out vec3 v_WorldPos;

            void main() {
                v_Color = a_Color;
                v_TexCoord = a_TexCoord;
                v_TexIndex = a_TexIndex;
                v_TilingFactor = a_TilingFactor;
                v_WorldPos = a_Position;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            layout(location = 0) out vec4 color;

            in vec4 v_Color;
            in vec2 v_TexCoord;
            in float v_TexIndex;
            in float v_TilingFactor;
            in vec3 v_WorldPos;

            uniform sampler2D u_Textures[32];
            uniform float u_AmbientIntensity = 0.1;
            uniform vec3 u_AmbientColor = vec3(1.0, 1.0, 1.0);
            
            struct PointLight {
                vec3 Position;
                vec3 Color;
                float Intensity;
                float Radius;
            };
            uniform int u_LightCount = 0;
            uniform PointLight u_Lights[8];

            uniform float u_VisibilityGrid[1024];
            uniform vec2 u_GridOrigin = vec2(0.0);
            uniform float u_CellSize = 1.0;
            uniform int u_GridSize = 32;
            uniform int u_UseVisibility = 0;

            struct Spotlight {
                vec3 Position;
                vec3 Direction;
                vec3 Color;
                float Intensity;
                float CutOff;
                float OuterCutOff;
            };
            uniform Spotlight u_Flashlight;
            uniform int u_FlashlightEnabled = 0;

            void main() {
                vec4 texColor = v_Color;
                int id = int(v_TexIndex);
                
                vec4 sampledColor;
                switch(id) {
                    case 0:  sampledColor = texture(u_Textures[0], v_TexCoord * v_TilingFactor); break;
                    case 1:  sampledColor = texture(u_Textures[1], v_TexCoord * v_TilingFactor); break;
                    case 2:  sampledColor = texture(u_Textures[2], v_TexCoord * v_TilingFactor); break;
                    case 3:  sampledColor = texture(u_Textures[3], v_TexCoord * v_TilingFactor); break;
                    case 4:  sampledColor = texture(u_Textures[4], v_TexCoord * v_TilingFactor); break;
                    case 5:  sampledColor = texture(u_Textures[5], v_TexCoord * v_TilingFactor); break;
                    case 6:  sampledColor = texture(u_Textures[6], v_TexCoord * v_TilingFactor); break;
                    case 7:  sampledColor = texture(u_Textures[7], v_TexCoord * v_TilingFactor); break;
                    case 8:  sampledColor = texture(u_Textures[8], v_TexCoord * v_TilingFactor); break;
                    case 9:  sampledColor = texture(u_Textures[9], v_TexCoord * v_TilingFactor); break;
                    case 10: sampledColor = texture(u_Textures[10], v_TexCoord * v_TilingFactor); break;
                    case 11: sampledColor = texture(u_Textures[11], v_TexCoord * v_TilingFactor); break;
                    case 12: sampledColor = texture(u_Textures[12], v_TexCoord * v_TilingFactor); break;
                    case 13: sampledColor = texture(u_Textures[13], v_TexCoord * v_TilingFactor); break;
                    case 14: sampledColor = texture(u_Textures[14], v_TexCoord * v_TilingFactor); break;
                    case 15: sampledColor = texture(u_Textures[15], v_TexCoord * v_TilingFactor); break;
                    case 16: sampledColor = texture(u_Textures[16], v_TexCoord * v_TilingFactor); break;
                    case 17: sampledColor = texture(u_Textures[17], v_TexCoord * v_TilingFactor); break;
                    case 18: sampledColor = texture(u_Textures[18], v_TexCoord * v_TilingFactor); break;
                    case 19: sampledColor = texture(u_Textures[19], v_TexCoord * v_TilingFactor); break;
                    case 20: sampledColor = texture(u_Textures[20], v_TexCoord * v_TilingFactor); break;
                    case 21: sampledColor = texture(u_Textures[21], v_TexCoord * v_TilingFactor); break;
                    case 22: sampledColor = texture(u_Textures[22], v_TexCoord * v_TilingFactor); break;
                    case 23: sampledColor = texture(u_Textures[23], v_TexCoord * v_TilingFactor); break;
                    case 24: sampledColor = texture(u_Textures[24], v_TexCoord * v_TilingFactor); break;
                    case 25: sampledColor = texture(u_Textures[25], v_TexCoord * v_TilingFactor); break;
                    case 26: sampledColor = texture(u_Textures[26], v_TexCoord * v_TilingFactor); break;
                    case 27: sampledColor = texture(u_Textures[27], v_TexCoord * v_TilingFactor); break;
                    case 28: sampledColor = texture(u_Textures[28], v_TexCoord * v_TilingFactor); break;
                    case 29: sampledColor = texture(u_Textures[29], v_TexCoord * v_TilingFactor); break;
                    case 30: sampledColor = texture(u_Textures[30], v_TexCoord * v_TilingFactor); break;
                    case 31: sampledColor = texture(u_Textures[31], v_TexCoord * v_TilingFactor); break;
                }
                
                texColor *= sampledColor;

                if (texColor.a < 0.1)
                    discard;
                
                // Color keying for pure white background (usually for sprite sheets)
                if (texColor.r > 0.999 && texColor.g > 0.999 && texColor.b > 0.999 && id > 0)
                    discard;

                vec3 diffuse = u_AmbientColor * u_AmbientIntensity;
                for(int i = 0; i < u_LightCount; i++) {
                    float dist = distance(v_WorldPos.xy, u_Lights[i].Position.xy);
                    if (dist < u_Lights[i].Radius) {
                        float atten = 1.0 - (dist / u_Lights[i].Radius);
                        diffuse += u_Lights[i].Color * u_Lights[i].Intensity * atten * atten;
                    }
                }

                if (u_FlashlightEnabled == 1) {
                    vec3 lightDir = normalize(u_Flashlight.Position - v_WorldPos);
                    float theta = dot(lightDir, normalize(-u_Flashlight.Direction));
                    float epsilon = u_Flashlight.CutOff - u_Flashlight.OuterCutOff;
                    float intensity = clamp((theta - u_Flashlight.OuterCutOff) / epsilon, 0.0, 1.0);
                    
                    float dist = distance(v_WorldPos.xy, u_Flashlight.Position.xy);
                    float atten = 1.0 / (1.0 + 0.04 * dist + 0.006 * dist * dist);
                    
                    
                    diffuse += u_Flashlight.Color * u_Flashlight.Intensity * intensity * atten;
                }

                float visibility = 1.0;
                if (u_UseVisibility == 1) {
                    int gx = int((v_WorldPos.x - u_GridOrigin.x) / u_CellSize);
                    int gy = int((v_WorldPos.y - u_GridOrigin.y) / u_CellSize);
                    if (gx >= 0 && gx < u_GridSize && gy >= 0 && gy < u_GridSize) {
                        visibility = u_VisibilityGrid[gy * u_GridSize + gx];
                    } else {
                        visibility = 0.0;
                    }
                }

                color = vec4(texColor.rgb * diffuse * visibility, texColor.a);
            }
        )";

        s_Data.TextureShader.reset(new Shader(vertexSrc, fragmentSrc));
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
    }

    void Renderer2D::Shutdown() {
        delete[] s_Data.QuadVertexBufferBase;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera) {
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
        s_Data.TextureShader->UploadUniformInt("u_LightCount", 0);

        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.TextureSlotIndex = 1;
        s_Data.LightCount = 0;
        s_Data.FlashlightEnabled = false;
        s_Data.TextureShader->UploadUniformInt("u_FlashlightEnabled", 0);
        s_Data.TextureShader->UploadUniformInt("u_UseVisibility", 0);
    }

    void Renderer2D::SetAmbientLight(float intensity, const glm::vec3& color) {
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformFloat("u_AmbientIntensity", intensity);
        s_Data.TextureShader->UploadUniformFloat3("u_AmbientColor", color);
    }

    void Renderer2D::SubmitLight(const glm::vec3& position, const glm::vec3& color, float intensity, float radius) {
        if (s_Data.LightCount >= 8) return;

        std::string base = "u_Lights[" + std::to_string(s_Data.LightCount) + "].";
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformFloat3(base + "Position", position);
        s_Data.TextureShader->UploadUniformFloat3(base + "Color", color);
        s_Data.TextureShader->UploadUniformFloat(base + "Intensity", intensity);
        s_Data.TextureShader->UploadUniformFloat(base + "Radius", radius);

        s_Data.LightCount++;
        s_Data.TextureShader->UploadUniformInt("u_LightCount", s_Data.LightCount);
    }

    void Renderer2D::SubmitSpotlight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float intensity, float cutoff, float outerCutoff) {
        s_Data.Flashlight = { position, direction, color, intensity, cutoff, outerCutoff };
        s_Data.FlashlightEnabled = true;

        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformFloat3("u_Flashlight.Position", position);
        s_Data.TextureShader->UploadUniformFloat3("u_Flashlight.Direction", direction);
        s_Data.TextureShader->UploadUniformFloat3("u_Flashlight.Color", color);
        s_Data.TextureShader->UploadUniformFloat("u_Flashlight.Intensity", intensity);
        s_Data.TextureShader->UploadUniformFloat("u_Flashlight.CutOff", glm::cos(glm::radians(cutoff)));
        s_Data.TextureShader->UploadUniformFloat("u_Flashlight.OuterCutOff", glm::cos(glm::radians(outerCutoff)));
        s_Data.TextureShader->UploadUniformInt("u_FlashlightEnabled", 1);
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformMat4("u_ViewProjection", viewProj);

        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::EndScene() {
        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
        s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
        Flush();
    }

    void Renderer2D::Flush() {
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->Bind(i);

        s_Data.QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data.QuadIndexCount, GL_UNSIGNED_INT, nullptr);
        s_Data.Stats.DrawCalls++;
    }

    void Renderer2D::SubmitVisibilityMap(const glm::vec2& gridOrigin, float cellSize, int gridSize, const float* data) {
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformInt("u_UseVisibility", 1);
        s_Data.TextureShader->UploadUniformFloat2("u_GridOrigin", gridOrigin);
        s_Data.TextureShader->UploadUniformFloat("u_CellSize", cellSize);
        s_Data.TextureShader->UploadUniformInt("u_GridSize", gridSize);
        s_Data.TextureShader->UploadUniformFloatArray("u_VisibilityGrid", data, gridSize * gridSize);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; 
        const float tilingFactor = 1.0f;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices) {
            EndScene();
            s_Data.QuadIndexCount = 0;
            s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
            s_Data.TextureSlotIndex = 1;
        }

        for (size_t i = 0; i < quadVertexCount; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
        if (!texture) {
            DrawQuad(transform, tintColor);
            return;
        }

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices) {
            EndScene();
            s_Data.QuadIndexCount = 0;
            s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
            s_Data.TextureSlotIndex = 1;
        }

        float textureIndex = -1.0f;
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
            if (*s_Data.TextureSlots[i].get() == *texture.get()) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == -1.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots) {
                 EndScene();
                 s_Data.QuadIndexCount = 0;
                 s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
                 s_Data.TextureSlotIndex = 1;
            }
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::ResetStats() {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats() {
        return s_Data.Stats;
    }

    std::shared_ptr<Shader> Renderer2D::GetShader() {
        return s_Data.TextureShader;
    }

}
