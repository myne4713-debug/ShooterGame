#include "world/Level.h"
#include "core/Config.h"
#include <algorithm>

Level::Level(MapId mapId) : Level(mapDefinition(mapId)) {}

Level::Level(const MapDefinition& def)
    : m_mapId(def.id),
      m_name(def.name),
      m_revision(def.revision),
      m_arenaSize(def.arenaSize),
      m_wallHeight(def.wallHeight),
      m_playerSpawn(def.playerSpawn),
      m_playerYaw(def.playerYaw),
      m_playerPitch(def.playerPitch),
      m_theme(def.theme),
      m_spawnPoints(def.spawnPoints),
      m_targetBaseColor(def.targetBox.material.color),
      m_targetHealth(config::kPracticeHealth) {
    const float halfX = m_arenaSize.x * 0.5f;
    const float halfZ = m_arenaSize.y * 0.5f;
    const float height = m_wallHeight;
    const float width = config::kWallThickness;
    const float edgeX = halfX + width * 0.5f;
    const float edgeZ = halfZ + width * 0.5f;

    // Tâm và kích thước dùng chung cho ma trận Model và hai góc min/max của AABB.
    m_boxes = {
        {{0.0f, height * 0.5f, -edgeZ}, {m_arenaSize.x + 2.0f * width, height, width}, def.wallMaterial},
        {{0.0f, height * 0.5f,  edgeZ}, {m_arenaSize.x + 2.0f * width, height, width}, def.wallMaterial},
        {{-edgeX, height * 0.5f, 0.0f}, {width, height, m_arenaSize.y}, def.wallMaterial},
        {{ edgeX, height * 0.5f, 0.0f}, {width, height, m_arenaSize.y}, def.wallMaterial},
    };
    m_boxes.insert(m_boxes.end(), def.interiorBoxes.begin(), def.interiorBoxes.end());
    m_boxes.push_back(def.targetBox);

    // Sàn ở chỉ số 0; sau đó các collider có cùng thứ tự với danh sách khối được vẽ.
    m_targetSolid = m_boxes.size();
    m_solids.push_back({{-halfX, 0.0f, -halfZ}, {halfX, 0.0f, halfZ}});
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
        m_boxes[index - 1].material.color = glm::mix(glm::vec3{0.22f}, m_targetBaseColor, fraction);
    }
    return hit;
}
