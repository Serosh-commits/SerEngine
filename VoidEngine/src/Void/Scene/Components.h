#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ScriptableEntity.h"

namespace Void {

    struct TransformComponent {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {}

        glm::mat4 GetTransform() const {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct SpriteRendererComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::shared_ptr<class Texture2D> Texture = nullptr;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
    };

    struct LightSourceComponent {
        float Intensity = 1.0f;
        float Radius = 5.0f;
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        bool IsFlickering = false;

        LightSourceComponent() = default;
        LightSourceComponent(const LightSourceComponent&) = default;
    };

    struct AIChaseComponent {
        enum class State { Idle, Searching, Chasing, Attacking };
        State CurrentState = State::Idle;

        float Speed = 1.0f;
        float DetectionRadius = 8.0f;
        float AttackRadius = 0.8f;
        
        float LoseTargetTimer = 0.0f;
        float MaxLoseTargetTime = 2.0f;

        AIChaseComponent() = default;
        AIChaseComponent(const AIChaseComponent&) = default;
    };

    struct FlashlightComponent {
        glm::vec3 Direction = { 1.0f, 0.0f, 0.0f };
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;
        float Cutoff = 12.5f;
        float OuterCutoff = 17.5f;
        bool Enabled = true;

        FlashlightComponent() = default;
        FlashlightComponent(const FlashlightComponent&) = default;
    };

    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity* (*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind() {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
        }
    };

    struct AudioComponent {
        std::string FilePath;
        bool IsPlaying = false;
        bool Loop = false;
        float Volume = 1.0f;
        float Pitch = 1.0f;
        
        float OcclusionVolume = 1.0f;

        AudioComponent() = default;
        AudioComponent(const AudioComponent&) = default;
        AudioComponent(const std::string& path) : FilePath(path) {}
    };

    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct BoxCollider2DComponent {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 0.5f, 0.5f };

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        bool IsStatic = false;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

}

#endif
