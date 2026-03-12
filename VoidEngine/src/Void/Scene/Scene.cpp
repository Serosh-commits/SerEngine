#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Void/Renderer/Renderer2D.h"
#include "Void/Physics/Physics2D.h"
#include "Void/Physics/PhysicsEngine.h"
#include "Void/Audio/AudioEngine.h"
#include "Void/Events/Event.h"

namespace Void {

    Scene::Scene() {}

    Scene::~Scene() {
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
            if (nsc.Instance) {
                nsc.Instance->OnDestroy();
                nsc.DestroyScript(&nsc);
            }
        });
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }

    void Scene::OnEvent(Event& e) {
        m_Registry.view<NativeScriptComponent>().each([&](auto entity, auto& nsc) {
            if (nsc.Instance)
                nsc.Instance->OnEvent(e);
        });
    }

    void Scene::OnUpdate(Timestep ts) {
        InitializeAndUpdateScripts(ts);
        PhysicsEngine::ResolveCollisions(this);
    }

    void Scene::OnRenderRuntime(Timestep ts, const OrthographicCamera& camera) {
        Renderer2D::BeginScene(camera);
        Renderer2D::SetAmbientLight(0.12f, { 0.5f, 0.5f, 0.7f });

        SubmitLights();
        SubmitSpotlights();
        RenderSprites();

        Renderer2D::EndScene();
    }

    bool Scene::Raycast(const glm::vec2& origin, const glm::vec2& direction,
                        float maxDistance, Physics2D::RaycastHit& outHit) {
        auto view = m_Registry.view<TransformComponent, BoxCollider2DComponent>();
        Physics2D::Ray ray = { origin, glm::normalize(direction) };
        float closestT = maxDistance;
        bool found = false;

        for (auto entity : view) {
            auto [transform, collider] = view.get<TransformComponent, BoxCollider2DComponent>(entity);
            
            if (!collider.IsStatic)
                continue;

            AABB bounds = {
                { transform.Translation.x - collider.Size.x, transform.Translation.y - collider.Size.y },
                { transform.Translation.x + collider.Size.x, transform.Translation.y + collider.Size.y }
            };

            float t;
            if (Physics2D::RayIntersectsAABB(ray, bounds, t) && t >= 0 && t < closestT) {
                closestT = t;
                found = true;
                outHit.Hit = true;
                outHit.Distance = t;
                outHit.Point = origin + ray.Direction * t;
            }
        }

        return found;
    }

    void Scene::InitializeAndUpdateScripts(Timestep ts) {
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
            if (!nsc.Instance) {
                nsc.Instance = nsc.InstantiateScript();
                nsc.Instance->m_Entity = Entity{ entity, this };
                nsc.Instance->OnCreate();
            }
            nsc.Instance->OnUpdate(ts);
        });
    }



    void Scene::SubmitLights() {
        auto view = m_Registry.view<TransformComponent, LightSourceComponent>();
        for (auto entity : view) {
            auto [transform, light] = view.get<TransformComponent, LightSourceComponent>(entity);
            float intensity = light.Intensity;
            if (light.IsFlickering) {
                float flicker = (rand() % 100) / 100.0f;
                intensity *= (0.8f + flicker * 0.2f);
            }
            Renderer2D::SubmitLight(transform.Translation, light.Color, intensity, light.Radius);
        }
    }

    void Scene::SubmitSpotlights() {
        auto view = m_Registry.view<TransformComponent, FlashlightComponent>();
        for (auto entity : view) {
            auto [transform, flashlight] = view.get<TransformComponent, FlashlightComponent>(entity);
            if (flashlight.Enabled) {
                Renderer2D::SubmitSpotlight(
                    transform.Translation, flashlight.Direction, flashlight.Color,
                    flashlight.Intensity, flashlight.Cutoff, flashlight.OuterCutoff
                );
            }
        }
    }

    void Scene::RenderSprites() {
        auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
        for (auto entity : view) {
            auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
            if (sprite.Texture)
                Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TilingFactor, sprite.Color);
            else
                Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
        }
    }

}
