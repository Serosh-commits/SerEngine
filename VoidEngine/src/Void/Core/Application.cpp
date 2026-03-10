#include "Application.h"
#include "Log.h"
#include "Void/Events/ApplicationEvent.h"
#include "Void/Audio/AudioEngine.h"
#include "Void/Renderer/Renderer2D.h"
#include "Void/Renderer/ResourceManager.h"
#include <SDL.h>
#include <epoxy/gl.h>
#include <functional>

namespace Void {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application* Application::s_Instance = nullptr;

    Application::Application() {
        s_Instance = this;
        Log::Init();
        AudioEngine::Init();
        VOID_INFO("VoidEngine Initialized - The Darkness Awaits.");

        m_Window = std::unique_ptr<Window>(Window::Create({"VoidEngine", 1280, 720}));
        m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
    }

    Application::~Application() {
        Renderer2D::Shutdown();
        ResourceManager::Shutdown();
        AudioEngine::Shutdown();
    }

    void Application::PushLayer(Layer* layer) {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* overlay) {
        m_LayerStack.PushOverlay(overlay);
    }

    void Application::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.Handled) break;
        }
    }

    void Application::Run() {
        while (m_Running) {
            float time = (float)SDL_GetTicks() / 1000.0f;
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        glViewport(0, 0, e.GetWidth(), e.GetHeight());
        return false;
    }

}
