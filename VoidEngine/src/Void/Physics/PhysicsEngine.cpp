#include "PhysicsEngine.h"
#include "Void/Scene/Scene.h"
#include "Void/Scene/Components.h"
#include <vector>
#include <cmath>

namespace Void {

    static constexpr int SOLVER_ITERATIONS = 4;

    static AABB ComputeBounds(const TransformComponent& transform, const BoxCollider2DComponent& collider) {
        return {
            { transform.Translation.x - collider.Size.x, transform.Translation.y - collider.Size.y },
            { transform.Translation.x + collider.Size.x, transform.Translation.y + collider.Size.y }
        };
    }

    static void SeparateBodies(TransformComponent& bodyA, const BoxCollider2DComponent& colA,
                               TransformComponent& bodyB, const BoxCollider2DComponent& colB) {
        float overlapX = (colA.Size.x + colB.Size.x) - std::abs(bodyA.Translation.x - bodyB.Translation.x);
        float overlapY = (colA.Size.y + colB.Size.y) - std::abs(bodyA.Translation.y - bodyB.Translation.y);

        bool resolveOnX = overlapX < overlapY;
        float overlap = resolveOnX ? overlapX : overlapY;
        float sign = resolveOnX
            ? (bodyA.Translation.x < bodyB.Translation.x ? -1.0f : 1.0f)
            : (bodyA.Translation.y < bodyB.Translation.y ? -1.0f : 1.0f);

        if (!colA.IsStatic) {
            if (resolveOnX) bodyA.Translation.x += sign * overlap;
            else            bodyA.Translation.y += sign * overlap;
        }
        if (!colB.IsStatic) {
            if (resolveOnX) bodyB.Translation.x -= sign * overlap;
            else            bodyB.Translation.y -= sign * overlap;
        }
    }

    void PhysicsEngine::ResolveCollisions(Scene* scene) {
        auto view = scene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();

        std::vector<entt::entity> entities;
        for (auto entity : view)
            entities.push_back(entity);

        for (int pass = 0; pass < SOLVER_ITERATIONS; pass++) {
            for (size_t i = 0; i < entities.size(); i++) {
                for (size_t j = i + 1; j < entities.size(); j++) {
                    auto& transformA = view.get<TransformComponent>(entities[i]);
                    auto& colliderA  = view.get<BoxCollider2DComponent>(entities[i]);
                    auto& transformB = view.get<TransformComponent>(entities[j]);
                    auto& colliderB  = view.get<BoxCollider2DComponent>(entities[j]);

                    if (colliderA.IsStatic && colliderB.IsStatic)
                        continue;

                    AABB boundsA = ComputeBounds(transformA, colliderA);
                    AABB boundsB = ComputeBounds(transformB, colliderB);

                    if (Physics2D::CheckCollision(boundsA, boundsB))
                        SeparateBodies(transformA, colliderA, transformB, colliderB);
                }
            }
        }
    }

}
