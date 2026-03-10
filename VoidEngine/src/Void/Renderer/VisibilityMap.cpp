#include "VisibilityMap.h"
#include "Void/Scene/Scene.h"
#include "Void/Physics/Physics2D.h"
#include <cmath>
#include <algorithm>

namespace Void {

    static constexpr int RAY_COUNT = 180;
    static constexpr float TWO_PI = 6.283185307f;

    void VisibilityMap::ComputeVisibility(const glm::vec2& origin, float viewRadius, Scene* scene) {
        m_GridOrigin = { origin.x - GRID_SIZE * CELL_SIZE * 0.5f,
                         origin.y - GRID_SIZE * CELL_SIZE * 0.5f };

        std::fill(m_Visibility.begin(), m_Visibility.end(), 0.0f);

        int centerGX = (int)((origin.x - m_GridOrigin.x) / CELL_SIZE);
        int centerGY = (int)((origin.y - m_GridOrigin.y) / CELL_SIZE);
        if (centerGX >= 0 && centerGX < GRID_SIZE && centerGY >= 0 && centerGY < GRID_SIZE) {
            m_Visibility[ToIndex(centerGX, centerGY)] = 1.0f;
            m_Explored[ToIndex(centerGX, centerGY)] = 1.0f;
        }

        for (int i = 0; i < RAY_COUNT; i++) {
            float angle = (float)i / (float)RAY_COUNT * TWO_PI;
            CastRay(origin, angle, viewRadius, scene);
        }
    }

    void VisibilityMap::CastRay(const glm::vec2& origin, float angle, float maxDist, Scene* scene) {
        glm::vec2 dir = { cos(angle), sin(angle) };
        float stepSize = CELL_SIZE * 0.5f;

        Physics2D::RaycastHit hit;
        float hitDist = maxDist;

        if (scene->Raycast(origin, dir, maxDist, hit))
            hitDist = std::min(hit.Distance + 0.5f, maxDist);

        for (float d = 0.0f; d < hitDist; d += stepSize) {
            glm::vec2 point = origin + dir * d;
            int gx = (int)((point.x - m_GridOrigin.x) / CELL_SIZE);
            int gy = (int)((point.y - m_GridOrigin.y) / CELL_SIZE);

            if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE)
                continue;

            float distRatio = d / maxDist;
            float brightness = 1.0f - distRatio * distRatio;

            int idx = ToIndex(gx, gy);
            m_Visibility[idx] = std::max(m_Visibility[idx], brightness);
            m_Explored[idx] = std::max(m_Explored[idx], 0.3f);
        }
    }

    float VisibilityMap::GetVisibility(float worldX, float worldY) const {
        int gx = (int)((worldX - m_GridOrigin.x) / CELL_SIZE);
        int gy = (int)((worldY - m_GridOrigin.y) / CELL_SIZE);
        if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE)
            return 0.0f;
        return m_Visibility[ToIndex(gx, gy)];
    }

    float VisibilityMap::GetExplored(float worldX, float worldY) const {
        int gx = (int)((worldX - m_GridOrigin.x) / CELL_SIZE);
        int gy = (int)((worldY - m_GridOrigin.y) / CELL_SIZE);
        if (gx < 0 || gx >= GRID_SIZE || gy < 0 || gy >= GRID_SIZE)
            return 0.0f;
        return m_Explored[ToIndex(gx, gy)];
    }

    int VisibilityMap::ToIndex(int gx, int gy) const {
        return gy * GRID_SIZE + gx;
    }

}
