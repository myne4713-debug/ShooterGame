#include "world/Level.h"
#include "core/Config.h"
#include <algorithm>

Level::Level() : m_targetHealth(config::kPracticeHealth) {
    const float half = config::kArenaSize * 0.5f;
    const float height = config::kWallHeight;
    const float width = config::kWallThickness;
    const float edge = half + width * 0.5f;
    const Material wall{Surface::Wall, {0.75f, 0.73f, 0.67f}, config::kWallShininess};
    const Material crate{Surface::Wood, {0.85f, 0.53f, 0.31f}, config::kWoodShininess};
    const Material pillar{Surface::Metal, {0.36f, 0.61f, 0.61f}, config::kMetalShininess};
    // Tâm và kích thước dùng chung cho ma trận Model và hai góc min/max của AABB.
    m_boxes = {
        {{0, height/2, -edge}, {config::kArenaSize + 2*width, height, width}, wall},
        {{0, height/2,  edge}, {config::kArenaSize + 2*width, height, width}, wall},
        {{-edge, height/2, 0}, {width, height, config::kArenaSize}, wall},
        {{ edge, height/2, 0}, {width, height, config::kArenaSize}, wall},
        {{-3, config::kLowCrateHeight / 2, 2}, {3, config::kLowCrateHeight, 2}, crate},
        {{3, 1.5f, -2}, {2, 3, 3}, pillar},
        {{-5, 2, -7}, {2, 4, 2}, pillar},
        {{1, 0.8f, -8}, {4, 1.6f, 2}, crate}
    };
    m_boxes.push_back({{0, 0.9f, 2}, {1.35f, 1.8f, 0.30f},
                       {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}});
    // Sàn ở chỉ số 0; sau đó các collider có cùng thứ tự với danh sách khối được vẽ.
    m_targetSolid = m_boxes.size();
    m_solids.push_back({{-half, 0, -half}, {half, 0, half}});
    for (const auto& box : m_boxes) {
        const glm::vec3 extent = box.size * 0.5f;
        m_solids.push_back({box.center - extent, box.center + extent});
    }
}

std::optional<ShotHit> Level::hitScan(glm::vec3 origin, glm::vec3 unitDirection, int damage,
                                      float maxDistance) {
    float nearest = maxDistance;
    std::optional<ShotHit> hit;
    std::size_t index = 0;
    for (std::size_t i = 0; i < m_solids.size(); ++i) {
        glm::vec3 normal;
        const auto t = rayVsAABB(origin, unitDirection, m_solids[i], &normal);
        // Tia có độ dài hướng bằng 1 nên t là khoảng cách trong hệ thế giới.
        if (t && *t <= nearest && (!hit || *t < nearest)) {
            nearest = *t;
            index = i;
            hit = ShotHit{origin + nearest * unitDirection, normal, m_solids[i], false};
        }
    }
    if (hit && index == m_targetSolid && m_targetHealth > 0) {
        m_targetHealth = std::max(0, m_targetHealth - damage);
        hit->damagedTarget = true;
        const float fraction = static_cast<float>(m_targetHealth) / config::kPracticeHealth;
        m_boxes[index - 1].material.color = glm::mix(glm::vec3{0.22f}, glm::vec3{0.96f, 0.57f, 0.13f}, fraction);
    }
    return hit;
}
