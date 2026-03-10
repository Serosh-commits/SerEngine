#ifndef VISIBILITY_MAP_H
#define VISIBILITY_MAP_H

#include <glm/glm.hpp>
#include <vector>
#include <cmath>

namespace Void {

    class Scene;

    class VisibilityMap {
    public:
        static constexpr int GRID_SIZE = 32;
        static constexpr float CELL_SIZE = 1.0f;

        VisibilityMap() {
            m_Visibility.resize(GRID_SIZE * GRID_SIZE, 0.0f);
            m_Explored.resize(GRID_SIZE * GRID_SIZE, 0.0f);
        }

        void ComputeVisibility(const glm::vec2& origin, float viewRadius, Scene* scene);
        float GetVisibility(float worldX, float worldY) const;
        float GetExplored(float worldX, float worldY) const;

        const glm::vec2& GetOrigin() const { return m_GridOrigin; }
        const std::vector<float>& GetVisibilityData() const { return m_Visibility; }

    private:
        void CastRay(const glm::vec2& origin, float angle, float maxDist, Scene* scene);
        int ToIndex(int gx, int gy) const;

        std::vector<float> m_Visibility;
        std::vector<float> m_Explored;
        glm::vec2 m_GridOrigin = { 0, 0 };
    };

}

#endif
