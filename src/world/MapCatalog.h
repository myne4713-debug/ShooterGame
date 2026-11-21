#pragma once

#include "gfx/Material.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class MapId { Arena, Warehouse, Citadel, Labyrinth, Custom };

inline constexpr std::size_t kMapCount = 4;

struct LevelBox {
    glm::vec3 center;
    glm::vec3 size;
    Material material;
};

struct PointLightSpec {
    glm::vec3 position;
    glm::vec3 color;
};

struct MapTheme {
    glm::vec3 skyHorizon;
    glm::vec3 skyZenith;
    glm::vec3 ambient;
    glm::vec3 sunDirection;
    glm::vec3 sunColor;
    float fogStart;
    float fogEnd;
    float shadowExtent;
    Material floorMaterial;
    std::array<PointLightSpec, 3> lights;
};

struct MapDefinition {
    MapId id;
    std::string name;
    std::string subtitle;
    std::uint64_t revision = 0;
    glm::vec2 arenaSize;
    float wallHeight;
    glm::vec3 playerSpawn;
    float playerYaw;
    float playerPitch;
    MapTheme theme;
    Material wallMaterial;
    std::vector<LevelBox> interiorBoxes;
    LevelBox targetBox;
    std::vector<glm::vec3> spawnPoints;
};

MapId mapIdFromIndex(std::size_t index);
std::size_t mapIndex(MapId id);
const MapDefinition& mapDefinition(MapId id);
const MapDefinition& mapDefinition(std::size_t index);
