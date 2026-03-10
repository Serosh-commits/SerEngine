#ifndef ENTITY_H
#define ENTITY_H

#include "Scene.h"
#include <entt.hpp>
#include "Void/Core/Log.h"

namespace Void {

    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            : m_EntityHandle(handle), m_Scene(scene) {}
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            if (HasComponent<T>()) {
                VOID_CORE_WARN("Entity already has component!");
            }
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent() {
            if (!HasComponent<T>()) {
               VOID_CORE_ERROR("Entity does not have component!");
            }
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent() {
            if (!HasComponent<T>()) {
                VOID_CORE_WARN("Entity does not have component!");
                return;
            }
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }

        Scene* GetScene() const { return m_Scene; }

    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };

}

#endif
