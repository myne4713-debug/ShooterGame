#pragma once
#include "physics/AABB.h"

// Các góc của vật cản là những điểm rẽ trên mặt sàn.
class ArenaRoutes {
public:
    explicit ArenaRoutes(const std::vector<AABB>& solids);
    void setGoal(glm::vec3 feet);
    glm::vec3 waypoint(glm::vec3 from) const;

private:
    bool clear(glm::vec3 from, glm::vec3 to) const;
    std::vector<AABB> m_obstacles;
    std::vector<glm::vec3> m_corners;
    std::vector<std::vector<float>> m_distances;
    std::vector<float> m_toGoal;
    glm::vec3 m_goal{};
    bool m_hasGoal = false;
};
