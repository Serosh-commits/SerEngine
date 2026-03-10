#ifndef SCENE_H
#define SCENE_H

#include "Void/Core/Timestep.h"
#include <entt.hpp>
#include "Void/Physics/Physics2D.h"
#include <glm/glm.hpp>

namespace Void {

    class Event;
    class Entity;
    struct TransformComponent;
    struct AIChaseComponent;
    class OrthographicCamera;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        void OnEvent(Event& e);
        void OnUpdate(Timestep ts);
        void OnRenderRuntime(Timestep ts, const class OrthographicCamera& camera);

        template<typename... Components>
        auto GetAllEntitiesWith() {
            return m_Registry.view<Components...>();
        }

        bool Raycast(const glm::vec2& origin, const glm::vec2& direction,
                     float maxDistance, Physics2D::RaycastHit& hit);

    private:
        void InitializeAndUpdateScripts(Timestep ts);
        void SubmitLights();
        void SubmitSpotlights();
        void RenderSprites();

        entt::registry m_Registry;
        friend class Entity;
    };

}

#endif
