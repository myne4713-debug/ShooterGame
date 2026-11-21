#pragma once
#include "core/Config.h"
#include "physics/AABB.h"
#include "world/MapCatalog.h"

#include <glm/glm.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct ShotHit {
    glm::vec3 point;
    glm::vec3 normal;
    AABB bounds;
    bool damagedTarget;
    bool movingTarget = false;
};

class Level {
public:
    explicit Level(MapId mapId = MapId::Arena);
    explicit Level(const MapDefinition& definition);
    MapId mapId() const { return m_mapId; }
    const std::string& mapName() const { return m_name; }
    std::uint64_t revision() const { return m_revision; }
    glm::vec2 arenaSize() const { return m_arenaSize; }
    float wallHeight() const { return m_wallHeight; }
    glm::vec3 playerSpawn() const { return m_playerSpawn; }
    float playerYaw() const { return m_playerYaw; }
    float playerPitch() const { return m_playerPitch; }
    const MapTheme& theme() const { return m_theme; }
    const std::vector<glm::vec3>& spawnPoints() const { return m_spawnPoints; }
    const std::vector<LevelBox>& boxes() const { return m_boxes; }
    const std::vector<AABB>& solids() const { return m_solids; }
    std::optional<ShotHit> hitScan(glm::vec3 origin, glm::vec3 unitDirection, int damage,
                                   float maxDistance = config::kShotRange);
    int targetHealth() const { return m_targetHealth; }

private:
    MapId m_mapId;
    std::string m_name;
    std::uint64_t m_revision = 0;
    glm::vec2 m_arenaSize;
    float m_wallHeight;
    glm::vec3 m_playerSpawn;
    float m_playerYaw;
    float m_playerPitch;
    MapTheme m_theme;
    std::vector<glm::vec3> m_spawnPoints;
    glm::vec3 m_targetBaseColor;
    std::vector<LevelBox> m_boxes;
    std::vector<AABB> m_solids;
    std::size_t m_targetSolid = 0;
    int m_targetHealth;
};
