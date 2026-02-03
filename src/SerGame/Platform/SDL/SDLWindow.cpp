#include "SDLWindow.h"
#include "SerGame/Events/ApplicationEvent.h"
#include "SerGame/Events/KeyEvent.h"
#include "SerGame/Events/MouseEvent.h"
#include "SerGame/Core/Log.h"

namespace SerGame {

    Window* Window::Create(const WindowProps& props) {
        return new SDLWindow(props);
    }

    SDLWindow::SDLWindow(const WindowProps& props) {
        Init(props);
    }

    SDLWindow::~SDLWindow() {
        Shutdown();
    }

    void SDLWindow::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        SER_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SER_CORE_ERROR("Could not initialize SDL! SDL_Error: {0}", SDL_GetError());
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        m_Window = SDL_CreateWindow(m_Data.Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Data.Width, m_Data.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

        if (!m_Window) {
            SER_CORE_ERROR("Could not create window! SDL_Error: {0}", SDL_GetError());
            return;
        }

        m_Context = SDL_GL_CreateContext(m_Window);
        SDL_GL_MakeCurrent(m_Window, m_Context);

        SetVSync(true);
    }

    void SDLWindow::Shutdown() {
        SDL_GL_DeleteContext(m_Context);
        SDL_DestroyWindow(m_Window);
        SDL_Quit();
    }

    void SDLWindow::OnUpdate() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    WindowCloseEvent e;
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        m_Data.Width = event.window.data1;
                        m_Data.Height = event.window.data2;
                        WindowResizeEvent e(m_Data.Width, m_Data.Height);
                        m_Data.EventCallback(e);
                    }
                    break;
                }
                case SDL_KEYDOWN: {
                    KeyPressedEvent e(event.key.keysym.sym, event.key.repeat);
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_KEYUP: {
                    KeyReleasedEvent e(event.key.keysym.sym);
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    MouseButtonPressedEvent e(event.button.button);
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    MouseButtonReleasedEvent e(event.button.button);
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_MOUSEMOTION: {
                    MouseMovedEvent e((float)event.motion.x, (float)event.motion.y);
                    m_Data.EventCallback(e);
                    break;
                }
                case SDL_MOUSEWHEEL: {
                    MouseScrolledEvent e((float)event.wheel.x, (float)event.wheel.y);
                    m_Data.EventCallback(e);
                    break;
                }
            }
        }

        SDL_GL_SwapWindow(m_Window);
    }

    void SDLWindow::SetVSync(bool enabled) {
        if (enabled)
            SDL_GL_SetSwapInterval(1);
        else
            SDL_GL_SetSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool SDLWindow::IsVSync() const {
        return m_Data.VSync;
    }

}
