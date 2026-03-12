#include <Void/Core/Application.h>
#include <Void/Core/EntryPoint.h>
#include <Void/Core/GameState.h>
#include <Void/Renderer/Framebuffer.h>
#include <Void/Core/Layer.h>
#include <Void/Renderer/Renderer2D.h>
#include <Void/Core/Input.h>
#include <Void/Renderer/PostProcessor.h>
#include <Void/Renderer/ParticleSystem.h>
#include <Void/Renderer/CameraEffects.h>
#include <Void/Scene/Scene.h>
#include <Void/Scene/Entity.h>
#include <Void/Scene/Components.h>
#include <Void/Audio/AudioEngine.h>
#include <Void/Physics/Physics2D.h>
#include <Void/Renderer/ResourceManager.h>
#include <Void/Renderer/VisibilityMap.h>
#include <Void/Scene/ScriptableEntity.h>
#include <SDL.h>
#include <epoxy/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>

static Void::GameData g_Game;
static Void::CameraShake g_Shake;

static const int MAP_WIDTH  = 24;
static const int MAP_HEIGHT = 18;

static const char g_LevelMap[MAP_HEIGHT][MAP_WIDTH + 1] = {
    "########################",
    "#......#.......#.......#",
    "#......#.......#.......#",
    "#..#...#...#...#...#...#",
    "#..#.......#.......#...#",
    "#..#####...####....#...#",
    "#..#...........#.......#",
    "#..#...........#.......#",
    "#......####....#...#####",
    "#......#..#............#",
    "#..#...#..#............#",
    "#..#...#..#....####....#",
    "#..#...#.......#..#....#",
    "#..#...........#..#....#",
    "#..######......#..#....#",
    "#..............#.......#",
    "#..............#.......#",
    "########################",
};

static constexpr float PLAYER_SPEED      = 3.0f;
static constexpr float FOOTSTEP_INTERVAL = 0.35f;
static constexpr float TOGGLE_COOLDOWN   = 0.3f;
static constexpr float BATTERY_DRAIN     = 0.5f;

class PlayerScript : public Void::ScriptableEntity {
public:
    void OnUpdate(Void::Timestep ts) override {
        if (g_Game.State != Void::GameState::Playing) return;

        float dt = (float)ts;
        auto& position = GetComponent<Void::TransformComponent>().Translation;
        auto& flashlight = GetComponent<Void::FlashlightComponent>();

        bool moving = HandleMovement(position, dt);
        AimFlashlight(flashlight);
        DrainBattery(flashlight, dt);
        HandleFlashlightToggle(flashlight, dt);
        ApplyFootstepShake(moving, dt);
    }

private:
    float m_ToggleCooldown = 0.5f;
    float m_FootstepTimer = 0.0f;

    bool HandleMovement(glm::vec3& position, float dt) {
        float speed = PLAYER_SPEED * dt;
        bool moved = false;

        if (Void::Input::IsKeyPressed(SDLK_w)) { position.y += speed; moved = true; }
        if (Void::Input::IsKeyPressed(SDLK_s)) { position.y -= speed; moved = true; }
        if (Void::Input::IsKeyPressed(SDLK_a)) { position.x -= speed; moved = true; }
        if (Void::Input::IsKeyPressed(SDLK_d)) { position.x += speed; moved = true; }

        return moved;
    }

    void AimFlashlight(Void::FlashlightComponent& fl) {
        auto& window = Void::Application::Get().GetWindow();
        float normalizedX = (Void::Input::GetMouseX() / (float)window.GetWidth()) * 2.0f - 1.0f;
        float normalizedY = (1.0f - Void::Input::GetMouseY() / (float)window.GetHeight()) * 2.0f - 1.0f;
        glm::vec3 aimDirection = glm::normalize(glm::vec3(normalizedX, normalizedY, 0.0f));
        fl.Direction = glm::normalize(glm::mix(fl.Direction, aimDirection, 0.15f));
    }

    void DrainBattery(Void::FlashlightComponent& fl, float dt) {
        if (!fl.Enabled || g_Game.BatteryLife <= 0.0f) return;

        g_Game.BatteryLife -= dt * BATTERY_DRAIN;
        if (g_Game.BatteryLife <= 0.0f) {
            g_Game.BatteryLife = 0.0f;
            fl.Enabled = false;
        }
    }

    void HandleFlashlightToggle(Void::FlashlightComponent& fl, float dt) {
        m_ToggleCooldown += dt;
        if (!Void::Input::IsKeyPressed(SDLK_f)) return;
        if (m_ToggleCooldown < TOGGLE_COOLDOWN) return;
        if (g_Game.BatteryLife <= 0.0f) return;

        fl.Enabled = !fl.Enabled;
        m_ToggleCooldown = 0.0f;
    }

    void ApplyFootstepShake(bool moving, float dt) {
        if (!moving) return;

        m_FootstepTimer += dt;
        if (m_FootstepTimer > FOOTSTEP_INTERVAL) {
            m_FootstepTimer = 0.0f;
            g_Shake.Trigger(0.008f, 0.08f, 60.0f);
        }
    }
};

static constexpr float CAMERA_LEFT    = -8.0f;
static constexpr float CAMERA_RIGHT   =  8.0f;
static constexpr float CAMERA_BOTTOM  = -4.5f;
static constexpr float CAMERA_TOP     =  4.5f;
static constexpr int   PARTICLE_POOL  = 1500;
static constexpr int   FBO_WIDTH      = 640;
static constexpr int   FBO_HEIGHT     = 360;

static constexpr float SANITY_DRAIN_RANGE  = 7.0f;
static constexpr float SANITY_DRAIN_RATE   = 6.0f;
static constexpr float SANITY_REGEN_RATE   = 3.0f;
static constexpr float ATTACK_DAMAGE_RATE  = 20.0f;
static constexpr float CAMERA_FOLLOW_SPEED = 6.0f;
static constexpr float DUST_EMIT_INTERVAL  = 0.04f;
static constexpr float STATS_LOG_INTERVAL  = 2.0f;

class MonsterScript : public Void::ScriptableEntity {
public:
    void OnUpdate(Void::Timestep ts) override {
        if (g_Game.State != Void::GameState::Playing) return;

        float dt = (float)ts;
        auto& transform = GetComponent<Void::TransformComponent>();
        auto& ai = GetComponent<Void::AIChaseComponent>();
        auto* scene = GetEntity().GetScene();

        glm::vec3 playerPos(0.0f);
        bool foundPlayer = false;
        auto view = scene->GetAllEntitiesWith<Void::TransformComponent, Void::TagComponent>();
        for (auto e : view) {
            auto [t, tag] = view.get<Void::TransformComponent, Void::TagComponent>(e);
            if (tag.Tag == "Player") {
                playerPos = t.Translation;
                foundPlayer = true;
                break;
            }
        }
        if (!foundPlayer) return;

        glm::vec3 toPlayer = playerPos - transform.Translation;
        float distance = glm::length(toPlayer);
        bool canSeePlayer = HasLineOfSight(transform.Translation, playerPos, scene);

        UpdateAIState(transform, ai, toPlayer, distance, canSeePlayer, dt);
    }

private:
    bool HasLineOfSight(const glm::vec3& from, const glm::vec3& to, Void::Scene* scene) {
        glm::vec2 direction = glm::vec2(to) - glm::vec2(from);
        float distance = glm::length(direction);

        Void::Physics2D::RaycastHit hit;
        if (!scene->Raycast(glm::vec2(from), direction, distance, hit))
            return true;

        return hit.Distance >= distance - 0.2f;
    }

    void UpdateAIState(Void::TransformComponent& transform, Void::AIChaseComponent& ai,
                       const glm::vec3& toPlayer, float distance,
                       bool canSeePlayer, float dt) {
        switch (ai.CurrentState) {
            case Void::AIChaseComponent::State::Idle:
                if (distance < ai.DetectionRadius && canSeePlayer) {
                    ai.CurrentState = Void::AIChaseComponent::State::Chasing;
                    ai.LoseTargetTimer = 0.0f;
                }
                break;
            case Void::AIChaseComponent::State::Chasing:
                if (canSeePlayer) {
                    ai.LoseTargetTimer = 0.0f;
                    GetComponent<Void::LightSourceComponent>().Intensity = 1.8f;
                    GetComponent<Void::LightSourceComponent>().Radius = 5.0f;
                    if (distance < ai.AttackRadius) {
                        ai.CurrentState = Void::AIChaseComponent::State::Attacking;
                    } else {
                        MoveToward(transform, toPlayer, ai.Speed * 1.5f * dt);
                    }
                } else {
                    GetComponent<Void::LightSourceComponent>().Intensity = 0.5f;
                    ai.LoseTargetTimer += dt;
                    if (ai.LoseTargetTimer >= ai.MaxLoseTargetTime) {
                        ai.CurrentState = Void::AIChaseComponent::State::Searching;
                        ai.LoseTargetTimer = 0.0f;
                    } else {
                        MoveToward(transform, toPlayer, ai.Speed * 0.7f * dt);
                    }
                }
                break;
            case Void::AIChaseComponent::State::Attacking:
                if (distance > ai.AttackRadius * 2.0f)
                    ai.CurrentState = Void::AIChaseComponent::State::Chasing;
                else if (distance > ai.AttackRadius)
                    MoveToward(transform, toPlayer, ai.Speed * 1.3f * dt);
                break;
            case Void::AIChaseComponent::State::Searching:
                if (canSeePlayer && distance < ai.DetectionRadius) {
                    ai.CurrentState = Void::AIChaseComponent::State::Chasing;
                    ai.LoseTargetTimer = 0.0f;
                } else {
                    MoveToward(transform, toPlayer, ai.Speed * 0.3f * dt);
                    ai.LoseTargetTimer += dt;
                    if (ai.LoseTargetTimer >= ai.MaxLoseTargetTime * 3.0f) {
                        ai.CurrentState = Void::AIChaseComponent::State::Idle;
                        ai.LoseTargetTimer = 0.0f;
                    }
                }
                break;
        }
    }

    void MoveToward(Void::TransformComponent& transform, const glm::vec3& direction, float amount) {
        if (glm::length(direction) > 0.01f)
            transform.Translation += glm::normalize(direction) * amount;
    }
};

class HorrorDevLayer : public Void::Layer {
public:
    HorrorDevLayer()
        : Layer("HorrorDevLayer"),
          m_Camera(CAMERA_LEFT, CAMERA_RIGHT, CAMERA_BOTTOM, CAMERA_TOP),
          m_Particles(PARTICLE_POOL) {
        Void::Renderer2D::Init();
        m_PostProcessor = std::make_shared<Void::PostProcessor>();
        m_Framebuffer = std::make_shared<Void::Framebuffer>(FBO_WIDTH, FBO_HEIGHT);
        m_Scene = std::make_shared<Void::Scene>();

        auto wallTex = Void::ResourceManager::LoadTexture("assets/textures/rusty_wall.png");
        auto playerTex = Void::ResourceManager::LoadTexture("assets/textures/player.png");

        CreatePlayer(playerTex);
        CreateMonster(playerTex);
        BuildLevel(wallTex);
        PlaceEnvironmentLights();
        CreatePostProcessShader();
        InitDustParticles();
    }

    void OnUpdate(Void::Timestep ts) override {
        float dt = (float)ts;
        m_Time += dt;

        UpdateGameState(dt);
        m_EscCooldown += dt;
        g_Shake.Update(dt);
        FollowPlayer(dt);
        UpdateVisibility();
        EmitDustParticles(dt);
        m_Particles.OnUpdate(dt);
        LogStats(dt);
        RenderFrame(ts);
    }

    void OnEvent(Void::Event& e) override {
        m_Scene->OnEvent(e);
    }

private:
    void CreatePlayer(const std::shared_ptr<Void::Texture2D>& texture) {
        m_Player = m_Scene->CreateEntity("Player");
        m_Player.GetComponent<Void::TransformComponent>().Translation = { 3.0f, 3.0f, 0.2f };
        m_Player.GetComponent<Void::TransformComponent>().Scale = { 0.7f, 0.9f, 1.0f };
        auto& sprite = m_Player.AddComponent<Void::SpriteRendererComponent>(glm::vec4{1,1,1,1});
        sprite.Texture = texture;
        auto& collider = m_Player.AddComponent<Void::BoxCollider2DComponent>();
        collider.Size = { 0.25f, 0.35f };
        m_Player.AddComponent<Void::NativeScriptComponent>().Bind<PlayerScript>();

        auto& light = m_Player.AddComponent<Void::LightSourceComponent>();
        light.Radius = 3.5f;
        light.Intensity = 0.6f;
        light.Color = { 0.9f, 0.85f, 0.7f };

        auto& flashlight = m_Player.AddComponent<Void::FlashlightComponent>();
        flashlight.Intensity = 2.5f;
        flashlight.Cutoff = 25.0f;
        flashlight.OuterCutoff = 45.0f;
    }

    void CreateMonster(const std::shared_ptr<Void::Texture2D>& texture) {
        m_Monster = m_Scene->CreateEntity("Monster");
        m_Monster.GetComponent<Void::TransformComponent>().Translation = { 8.0f, 6.0f, 0.15f };
        m_Monster.GetComponent<Void::TransformComponent>().Scale = { 0.6f, 0.95f, 1.0f };
        auto& sprite = m_Monster.AddComponent<Void::SpriteRendererComponent>(glm::vec4{0.7f, 0.05f, 0.05f, 1});
        sprite.Texture = texture;

        m_Monster.AddComponent<Void::NativeScriptComponent>().Bind<MonsterScript>();

        auto& ai = m_Monster.AddComponent<Void::AIChaseComponent>();
        ai.Speed = 1.4f;
        ai.DetectionRadius = 14.0f;
        ai.AttackRadius = 0.7f;

        auto& collider = m_Monster.AddComponent<Void::BoxCollider2DComponent>();
        collider.Size = { 0.22f, 0.38f };

        auto& light = m_Monster.AddComponent<Void::LightSourceComponent>();
        light.Color = { 0.9f, 0.05f, 0.0f };
        light.Intensity = 0.5f;
        light.Radius = 3.0f;
        light.IsFlickering = true;
    }

    void BuildLevel(const std::shared_ptr<Void::Texture2D>& wallTexture) {
        for (int row = 0; row < MAP_HEIGHT; row++) {
            for (int col = 0; col < MAP_WIDTH; col++) {
                float worldX = (float)col;
                float worldY = (float)(MAP_HEIGHT - 1 - row);

                if (g_LevelMap[row][col] == '#')
                    CreateWall(worldX, worldY, col, row, wallTexture);
                else
                    CreateFloor(worldX, worldY, col, row);
            }
        }
    }

    void CreateWall(float x, float y, int col, int row, const std::shared_ptr<Void::Texture2D>& texture) {
        auto wall = m_Scene->CreateEntity("Wall");
        wall.GetComponent<Void::TransformComponent>().Translation = { x, y, -0.1f };

        float shade = 0.12f + (float)((col * 7 + row * 13) % 10) * 0.012f;
        auto& sprite = wall.AddComponent<Void::SpriteRendererComponent>(
            glm::vec4{ shade, shade * 0.9f, shade * 0.82f, 1.0f }
        );
        sprite.Texture = texture;

        auto& collider = wall.AddComponent<Void::BoxCollider2DComponent>();
        collider.IsStatic = true;
        collider.Size = { 0.48f, 0.48f };
    }

    void CreateFloor(float x, float y, int col, int row) {
        auto floor = m_Scene->CreateEntity("Floor");
        floor.GetComponent<Void::TransformComponent>().Translation = { x, y, -0.2f };

        float shade = 0.06f + (float)((col * 3 + row * 7) % 8) * 0.008f;
        floor.AddComponent<Void::SpriteRendererComponent>(
            glm::vec4{ shade, shade * 0.85f, shade * 0.75f, 1.0f }
        );
    }

    void PlaceEnvironmentLights() {
        PlaceLight(6.0f,  8.0f,  { 0.85f, 0.70f, 0.40f }, 0.8f, 6.0f);
        PlaceLight(16.0f, 4.0f,  { 0.30f, 0.80f, 0.40f }, 0.5f, 5.0f);
        PlaceLight(12.0f, 14.0f, { 0.90f, 0.25f, 0.10f }, 0.5f, 5.0f);
        PlaceLight(20.0f, 12.0f, { 0.40f, 0.40f, 0.90f }, 0.4f, 4.5f);
    }

    void PlaceLight(float x, float y, glm::vec3 color, float intensity, float radius) {
        auto entity = m_Scene->CreateEntity("Light");
        entity.GetComponent<Void::TransformComponent>().Translation = { x, y, 0.5f };
        auto& light = entity.AddComponent<Void::LightSourceComponent>();
        light.Color = color;
        light.Intensity = intensity;
        light.Radius = radius;
        light.IsFlickering = true;
    }

    void CreatePostProcessShader() {
        std::string vertexSource = R"(
            #version 330 core
            layout(location=0) in vec2 a_Position;
            layout(location=1) in vec2 a_TexCoord;
            out vec2 v_TexCoord;
            void main() {
                v_TexCoord = a_TexCoord;
                gl_Position = vec4(a_Position, 0.0, 1.0);
            }
        )";

        std::string fragmentSource = R"(
            #version 330 core
            layout(location=0) out vec4 color;
            in vec2 v_TexCoord;
            uniform sampler2D u_Texture;
            uniform float u_Time;
            uniform float u_Sanity;
            uniform float u_Health;

            float hash(vec2 p) {
                return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
            }

            void main() {
                vec2 uv = v_TexCoord;
                float sanity = clamp(u_Sanity / 100.0, 0.0, 1.0);
                float insanity = 1.0 - sanity;

                float wobble = 0.0003 + insanity * 0.002;
                uv.x += sin(u_Time * 1.5 + uv.y * 12.0) * wobble;

                float aberration = 0.001 + insanity * 0.004;
                aberration += aberration * 0.3 * sin(u_Time * 0.7);
                float r = texture(u_Texture, uv + vec2(aberration, 0.0)).r;
                float g = texture(u_Texture, uv).g;
                float b = texture(u_Texture, uv - vec2(aberration, 0.0)).b;
                vec3 col = vec3(r, g, b);

                col -= sin(uv.y * 900.0) * 0.012;
                col += (hash(uv + fract(u_Time)) - 0.5) * (0.025 + insanity * 0.07);

                float vignetteRadius = distance(uv, vec2(0.5));
                col *= smoothstep(1.1, 0.55 + sanity * 0.15, vignetteRadius);

                if (u_Health < 35.0) {
                    float pulse = sin(u_Time * 5.0) * 0.5 + 0.5;
                    col.r += 0.06 * pulse * (1.0 - u_Health / 35.0);
                }

                color = vec4(col, 1.0);
            }
        )";

        m_PostProcessShader = Void::ResourceManager::LoadShaderFromSrc("PostProcess", vertexSource, fragmentSource);
    }

    void InitDustParticles() {
        m_DustProps.ColorBegin = { 0.5f, 0.45f, 0.35f, 0.12f };
        m_DustProps.ColorEnd = { 0.3f, 0.25f, 0.2f, 0.0f };
        m_DustProps.SizeBegin = 0.06f;
        m_DustProps.SizeEnd = 0.01f;
        m_DustProps.SizeVariation = 0.03f;
        m_DustProps.LifeTime = 5.0f;
        m_DustProps.VelocityVariation = { 0.2f, 0.1f };
    }

    void UpdateGameState(float dt) {
        switch (g_Game.State) {
            case Void::GameState::Playing:
                UpdatePlaying(dt);
                break;
            case Void::GameState::Paused:
                if (Void::Input::IsKeyPressed(SDLK_ESCAPE) && m_EscCooldown > 0.4f) {
                    g_Game.State = Void::GameState::Playing;
                    m_EscCooldown = 0.0f;
                }
                break;
            case Void::GameState::GameOver:
                if (Void::Input::IsKeyPressed(SDLK_r))
                    RestartGame();
                break;
            default:
                break;
        }
    }

    void UpdatePlaying(float dt) {
        Void::Timestep ts(dt);
        m_Scene->OnUpdate(ts);
        g_Game.PlayTime += dt;

        glm::vec2 playerPos = glm::vec2(m_Player.GetComponent<Void::TransformComponent>().Translation);
        glm::vec2 monsterPos = glm::vec2(m_Monster.GetComponent<Void::TransformComponent>().Translation);
        float monsterDistance = glm::distance(playerPos, monsterPos);

        Void::AudioEngine::SetListenerPosition(glm::vec3(playerPos.x, playerPos.y, 0.0f));

        UpdateSanity(monsterDistance, dt);
        UpdateHealth(dt);

        if (Void::Input::IsKeyPressed(SDLK_ESCAPE) && m_EscCooldown > 0.4f) {
            g_Game.State = Void::GameState::Paused;
            m_EscCooldown = 0.0f;
        }
    }

    void UpdateSanity(float monsterDistance, float dt) {
        if (monsterDistance < SANITY_DRAIN_RANGE) {
            float proximity = 1.0f - monsterDistance / SANITY_DRAIN_RANGE;
            g_Game.Sanity -= proximity * SANITY_DRAIN_RATE * dt;
            g_Game.Sanity = std::max(g_Game.Sanity, 0.0f);
        } else {
            g_Game.Sanity = std::min(g_Game.Sanity + SANITY_REGEN_RATE * dt, g_Game.MaxSanity);
        }
    }

    void UpdateHealth(float dt) {
        auto& ai = m_Monster.GetComponent<Void::AIChaseComponent>();
        if (ai.CurrentState != Void::AIChaseComponent::State::Attacking) return;

        g_Game.Health -= ATTACK_DAMAGE_RATE * dt;
        if (!g_Shake.IsActive())
            g_Shake.Trigger(0.25f, 0.45f, 90.0f);

        if (g_Game.Health <= 0.0f) {
            g_Game.Health = 0.0f;
            g_Game.State = Void::GameState::GameOver;
            g_Shake.Trigger(0.8f, 1.2f, 15.0f);
        }
    }

    void RestartGame() {
        g_Game = Void::GameData{};
        m_Player.GetComponent<Void::TransformComponent>().Translation = { 3.0f, 3.0f, 0.2f };
        m_Monster.GetComponent<Void::TransformComponent>().Translation = { 8.0f, 6.0f, 0.15f };
        m_Monster.GetComponent<Void::AIChaseComponent>().CurrentState = Void::AIChaseComponent::State::Idle;
    }

    void FollowPlayer(float dt) {
        auto& playerPos = m_Player.GetComponent<Void::TransformComponent>().Translation;
        glm::vec3 cameraPos = m_Camera.GetPosition();
        cameraPos.x += (playerPos.x - cameraPos.x) * CAMERA_FOLLOW_SPEED * dt;
        cameraPos.y += (playerPos.y - cameraPos.y) * CAMERA_FOLLOW_SPEED * dt;

        auto shakeOffset = g_Shake.GetOffset();
        m_Camera.SetPosition({ cameraPos.x + shakeOffset.x, cameraPos.y + shakeOffset.y, 0.0f });
    }

    void UpdateVisibility() {
        if (g_Game.State != Void::GameState::Playing) return;
        auto& playerPos = m_Player.GetComponent<Void::TransformComponent>().Translation;
        m_VisibilityMap.ComputeVisibility({ playerPos.x, playerPos.y }, 12.0f, m_Scene.get());
    }

    void EmitDustParticles(float dt) {
        m_DustTimer += dt;
        if (m_DustTimer < DUST_EMIT_INTERVAL) return;

        m_DustTimer = 0.0f;
        auto& playerPos = m_Player.GetComponent<Void::TransformComponent>().Translation;
        m_DustProps.Position = {
            playerPos.x + ((rand() % 100) / 50.0f - 1.0f) * 3.0f,
            playerPos.y + ((rand() % 100) / 50.0f - 1.0f) * 2.0f
        };
        m_DustProps.Velocity = { 0.04f, 0.015f };
        m_Particles.Emit(m_DustProps);
    }

    void LogStats(float dt) {
        m_StatsTimer += dt;
        if (m_StatsTimer < STATS_LOG_INTERVAL) return;

        auto stats = Void::Renderer2D::GetStats();
        VOID_INFO("R2D: {0} DC, {1} Q | HP:{2:.0f} SAN:{3:.0f} BAT:{4:.0f}",
            stats.DrawCalls, stats.QuadCount, g_Game.Health, g_Game.Sanity, g_Game.BatteryLife);
        Void::Renderer2D::ResetStats();
        m_StatsTimer = 0.0f;
    }

    void RenderFrame(Void::Timestep ts) {
        GLint savedViewport[4];
        glGetIntegerv(GL_VIEWPORT, savedViewport);

        m_Framebuffer->Bind();
        glViewport(0, 0, FBO_WIDTH, FBO_HEIGHT);
        glClearColor(0.002f, 0.002f, 0.006f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Void::Renderer2D::SubmitVisibilityMap(m_VisibilityMap.GetOrigin(), 
                                              Void::VisibilityMap::CELL_SIZE,
                                              Void::VisibilityMap::GRID_SIZE,
                                              m_VisibilityMap.GetVisibilityData().data());

        m_Scene->OnRenderRuntime(ts, m_Camera);
        m_Particles.OnRender();
        DrawHUD();

        m_Framebuffer->Unbind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_PostProcessShader->Bind();
        m_PostProcessShader->UploadUniformFloat("u_Time", m_Time);
        m_PostProcessShader->UploadUniformFloat("u_Sanity", g_Game.Sanity);
        m_PostProcessShader->UploadUniformFloat("u_Health", g_Game.Health);
        m_PostProcessor->Render(m_Framebuffer, m_PostProcessShader);
    }

    void DrawHUD() {
        auto& cameraPos = m_Camera.GetPosition();
        float barX = cameraPos.x - 7.2f;
        float barY = cameraPos.y + 4.0f;
        float barWidth = 2.5f;
        float barHeight = 0.18f;

        DrawBar(barX, barY,          barWidth, barHeight, g_Game.Health    / g_Game.MaxHealth,      {0.15f, 0, 0, 0.5f},     {0.75f, 0.08f, 0.08f, 0.75f});
        DrawBar(barX, barY - 0.24f,  barWidth, barHeight, g_Game.Sanity    / g_Game.MaxSanity,      {0, 0.05f, 0.15f, 0.5f}, {0.15f, 0.35f, 0.85f, 0.75f});
        DrawBar(barX, barY - 0.48f,  barWidth, barHeight, g_Game.BatteryLife / g_Game.MaxBatteryLife, {0.1f, 0.1f, 0, 0.5f},   {0.85f, 0.75f, 0.15f, 0.75f});

        if (g_Game.State == Void::GameState::Paused)
            Void::Renderer2D::DrawQuad({ cameraPos.x, cameraPos.y, 0.95f }, { 16.0f, 9.0f }, { 0, 0, 0, 0.65f });

        if (g_Game.State == Void::GameState::GameOver)
            Void::Renderer2D::DrawQuad({ cameraPos.x, cameraPos.y, 0.95f }, { 16.0f, 9.0f }, { 0.12f, 0, 0, 0.8f });
    }

    void DrawBar(float x, float y, float width, float height, float ratio,
                 const glm::vec4& bgColor, const glm::vec4& fgColor) {
        Void::Renderer2D::DrawQuad({ x, y, 0.9f }, { width, height }, bgColor);
        if (ratio > 0.001f)
            Void::Renderer2D::DrawQuad({ x - width * (1.0f - ratio) * 0.5f, y, 0.91f }, { width * ratio, height }, fgColor);
    }

    std::shared_ptr<Void::Shader> m_PostProcessShader;
    std::shared_ptr<Void::PostProcessor> m_PostProcessor;
    std::shared_ptr<Void::Framebuffer> m_Framebuffer;
    std::shared_ptr<Void::Scene> m_Scene;
    Void::Entity m_Player;
    Void::Entity m_Monster;
    Void::OrthographicCamera m_Camera;
    Void::ParticleSystem m_Particles;
    Void::ParticleProps m_DustProps;
    Void::VisibilityMap m_VisibilityMap;
    float m_Time       = 0.0f;
    float m_StatsTimer = 0.0f;
    float m_DustTimer  = 0.0f;
    float m_EscCooldown = 0.5f;
};

class Sandbox : public Void::Application {
public:
    Sandbox() { PushLayer(new HorrorDevLayer()); }
    ~Sandbox() {}
};

Void::Application* Void::CreateApplication() {
    return new Sandbox();
}
