#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>

namespace SerGame {

    class Input {
    public:
        static bool IsKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };

}

#endif
