#include "world/ArenaRoutes.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <limits>

namespace {
constexpr float kMargin = 0.06f;
constexpr float kInfinity = std::numeric_limits<float>::infinity();

bool insideXZ(glm::vec3 point, const AABB& box) {
    return point.x >= box.min.x && point.x <= box.max.x
        && point.z >= box.min.z && point.z <= box.max.z;
}
}

ArenaRoutes::ArenaRoutes(const std::vector<AABB>& solids, glm::vec2 arenaSize) {
    const float limitX = arenaSize.x * 0.5f - config::kEnemyRadius;
    const float limitZ = arenaSize.y * 0.5f - config::kEnemyRadius;
    const glm::vec3 margin{config::kEnemyRadius, 0, config::kEnemyRadius};
    for (const AABB& box : solids) {
        if (box.max.y <= 0) continue;
        // Nới vật cản theo bán kính thân để một đoạn thẳng thông thoáng cũng đủ rộng cho địch.
        m_obstacles.push_back({box.min - margin, box.max + margin});
    }
    for (const AABB& expanded : m_obstacles) {
        for (float x : {expanded.min.x - kMargin, expanded.max.x + kMargin}) {
            for (float z : {expanded.min.z - kMargin, expanded.max.z + kMargin}) {
                if (std::abs(x) >= limitX || std::abs(z) >= limitZ) continue;
                const glm::vec3 corner{x, 0, z};
                // Bỏ góc nằm lọt trong AABB mở rộng khác (ví dụ bệ nhiều bậc) để tránh đỉnh thừa.
                const bool occluded = std::any_of(m_obstacles.begin(), m_obstacles.end(),
                                                  [&](const AABB& obs) { return insideXZ(corner, obs); });
                if (!occluded) m_corners.push_back(corner);
            }
        }
    }
    const auto count = m_corners.size();
    m_distances.assign(count, std::vector<float>(count, kInfinity));
    for (std::size_t i = 0; i < count; ++i) {
        for (std::size_t j = 0; j < count; ++j) {
            if (clear(m_corners[i], m_corners[j])) {
                m_distances[i][j] = glm::distance(m_corners[i], m_corners[j]);
            }
        }
    }
    // Floyd–Warshall chỉ chạy lúc tạo bản đồ. Thử đi qua từng góc k để rút ngắn đường i → j.
    for (std::size_t k = 0; k < count; ++k)
        for (std::size_t i = 0; i < count; ++i)
            for (std::size_t j = 0; j < count; ++j)
                m_distances[i][j] = std::min(m_distances[i][j], m_distances[i][k] + m_distances[k][j]);
    m_toGoal.resize(count);
}

bool ArenaRoutes::clear(glm::vec3 from, glm::vec3 to) const {
    for (const auto& box : m_obstacles) {
        const auto t = rayVsAABB(from, to - from, box);
        if (t && *t <= 1) return false;
    }
    return true;
}

void ArenaRoutes::setGoal(glm::vec3 feet) {
    feet.y = 0;
    // Khi người chơi đứng trên nắp thùng hoặc sát tường, hướng tới mép gần nhất mà thân địch tới được.
    for (int pass = 0; pass < 2; ++pass) {
        bool adjusted = false;
        for (const auto& box : m_obstacles) {
            if (!insideXZ(feet, box)) continue;
            const float gaps[]{feet.x - box.min.x, box.max.x - feet.x, feet.z - box.min.z, box.max.z - feet.z};
            const auto face = std::min_element(gaps, gaps + 4) - gaps;
            if (face == 0) feet.x = box.min.x - kMargin;
            if (face == 1) feet.x = box.max.x + kMargin;
            if (face == 2) feet.z = box.min.z - kMargin;
            if (face == 3) feet.z = box.max.z + kMargin;
            adjusted = true;
        }
        if (!adjusted) break;
    }
    if (m_hasGoal && feet == m_goal) return;
    m_goal = feet;
    m_hasGoal = true;
    std::fill(m_toGoal.begin(), m_toGoal.end(), kInfinity);
    for (std::size_t j = 0; j < m_corners.size(); ++j) {
        if (!clear(m_corners[j], feet)) continue;
        const float last = glm::distance(m_corners[j], feet);
        for (std::size_t i = 0; i < m_corners.size(); ++i)
            m_toGoal[i] = std::min(m_toGoal[i], m_distances[i][j] + last);
    }
}

glm::vec3 ArenaRoutes::waypoint(glm::vec3 from) const {
    if (clear(from, m_goal)) return m_goal;
    glm::vec3 next = from;
    float best = kInfinity;
    for (std::size_t i = 0; i < m_corners.size(); ++i) {
        const float first = glm::distance(from, m_corners[i]);
        const float total = first + m_toGoal[i];
        if (first > kMargin / 2 && total < best && clear(from, m_corners[i])) {
            next = m_corners[i];
            best = total;
        }
    }
    return next;
}
