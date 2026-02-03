#ifndef APPLICATION_H
#define APPLICATION_H

#include "SerGame/Core/LayerStack.h"
#include "SerGame/Core/Window.h"
#include "SerGame/Events/ApplicationEvent.h"
#include <memory>

namespace SerGame {

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline static Application& Get() { return *s_Instance; }
        inline Window& GetWindow() { return *m_Window; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

        static Application* s_Instance;
    };

    Application* CreateApplication();

}

#endif
