#include "Input.h"
#include "Application.h"
#include <SDL.h>

namespace SerGame {

    bool Input::IsKeyPressed(int keycode) {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
        return state[scancode];
    }

    bool Input::IsMouseButtonPressed(int button) {
        int x, y;
        uint32_t state = SDL_GetMouseState(&x, &y);
        return state & SDL_BUTTON(button);
    }

    glm::vec2 Input::GetMousePosition() {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return { (float)x, (float)y };
    }

    float Input::GetMouseX() { return GetMousePosition().x; }
    float Input::GetMouseY() { return GetMousePosition().y; }

}
