#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "Physics2D.h"
#include <glm/glm.hpp>

namespace Void {

    class PhysicsEngine {
    public:
        static void ResolveCollisions(class Scene* scene);
    };

}

#endif
