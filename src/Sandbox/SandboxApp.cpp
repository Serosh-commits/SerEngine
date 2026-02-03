#include <SerGame/Core/Application.h>
#include <SDL.h>
#include <SerGame/Core/EntryPoint.h>
#include <SerGame/Core/Input.h>
#include <SerGame/Core/Log.h>
#include <SerGame/Renderer/Renderer.h>
#include <SerGame/Renderer/VertexArray.h>
#include <SerGame/Renderer/Shader.h>
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public SerGame::Layer {
public:
    ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) {
        m_VertexArray.reset(SerGame::VertexArray::Create());

        float vertices[3 * 3] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        std::shared_ptr<SerGame::VertexBuffer> vertexBuffer;
        vertexBuffer.reset(SerGame::VertexBuffer::Create(vertices, sizeof(vertices)));
        vertexBuffer->SetLayout({ { SerGame::ShaderDataType::Float3, "a_Position" } });
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = { 0, 1, 2 };
        std::shared_ptr<SerGame::IndexBuffer> indexBuffer;
        indexBuffer.reset(SerGame::IndexBuffer::Create(indices, 3));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            uniform mat4 u_ViewProjection;
            uniform mat4 u_Transform;
            void main() {
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            layout(location = 0) out vec4 color;
            void main() {
                color = vec4(0.8, 0.2, 0.3, 1.0);
            }
        )";

        m_Shader.reset(new SerGame::Shader(vertexSrc, fragmentSrc));
    }

    void OnUpdate(SerGame::Timestep ts) override {
        if (SerGame::Input::IsKeyPressed(SDLK_LEFT))
            m_CameraPosition.x -= m_CameraSpeed * ts;
        else if (SerGame::Input::IsKeyPressed(SDLK_RIGHT))
            m_CameraPosition.x += m_CameraSpeed * ts;

        if (SerGame::Input::IsKeyPressed(SDLK_UP))
            m_CameraPosition.y += m_CameraSpeed * ts;
        else if (SerGame::Input::IsKeyPressed(SDLK_DOWN))
            m_CameraPosition.y -= m_CameraSpeed * ts;

        m_Camera.SetPosition(m_CameraPosition);

        SerGame::Renderer::BeginScene(m_Camera);
        SerGame::Renderer::Submit(m_VertexArray, m_Shader);
        SerGame::Renderer::EndScene();
    }

private:
    std::shared_ptr<SerGame::Shader> m_Shader;
    std::shared_ptr<SerGame::VertexArray> m_VertexArray;
    
    SerGame::OrthographicCamera m_Camera;
    glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
    float m_CameraSpeed = 2.0f;
};

class Sandbox : public SerGame::Application {
public:
    Sandbox() { PushLayer(new ExampleLayer()); }
    ~Sandbox() {}
};

SerGame::Application* SerGame::CreateApplication() {
    return new Sandbox();
}
