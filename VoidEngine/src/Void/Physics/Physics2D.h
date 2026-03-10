#ifndef PHYSICS2D_H
#define PHYSICS2D_H

#include <glm/glm.hpp>
#include <algorithm>

namespace Void {

    struct AABB {
        glm::vec2 Min;
        glm::vec2 Max;
    };

    class Physics2D {
    public:
        struct Ray {
            glm::vec2 Origin;
            glm::vec2 Direction;
        };

        struct RaycastHit {
            bool Hit = false;
            float Distance = 0.0f;
            glm::vec2 Point = {0.0f, 0.0f};
            glm::vec2 Normal = {0.0f, 0.0f};
        };

        static bool CheckCollision(const AABB& a, const AABB& b) {
            return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
                   (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y);
        }

        static bool RayIntersectsAABB(const Ray& ray, const AABB& aabb, float& t) {
            float t1 = (aabb.Min.x - ray.Origin.x) / ray.Direction.x;
            float t2 = (aabb.Max.x - ray.Origin.x) / ray.Direction.x;
            float t3 = (aabb.Min.y - ray.Origin.y) / ray.Direction.y;
            float t4 = (aabb.Max.y - ray.Origin.y) / ray.Direction.y;

            float tmin = std::max(std::min(t1, t2), std::min(t3, t4));
            float tmax = std::min(std::max(t1, t2), std::max(t3, t4));

            if (tmax < 0 || tmin > tmax) return false;
            t = tmin;
            return true;
        }
    };

}

#endif
