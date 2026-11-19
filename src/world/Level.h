#pragma once
#include "core/Config.h"
#include "gfx/Material.h"
#include "physics/AABB.h"

#include <glm/glm.hpp>
#include <vector>

struct LevelBox {
    glm::vec3 center;
    glm::vec3 size;
    Material material;
};

struct ShotHit {
    glm::vec3 point;
    glm::vec3 normal;
    AABB bounds;
    bool damagedTarget;
    bool movingTarget = false;
};

class Level {
public:
    Level();
    const std::vector<LevelBox>& boxes() const { return m_boxes; }
    const std::vector<AABB>& solids() const { return m_solids; }
    std::optional<ShotHit> hitScan(glm::vec3 origin, glm::vec3 unitDirection, int damage,
                                   float maxDistance = config::kShotRange);
    int targetHealth() const { return m_targetHealth; }

private:
    std::vector<LevelBox> m_boxes;
    std::vector<AABB> m_solids;
    std::size_t m_targetSolid = 0;
    int m_targetHealth;
};
