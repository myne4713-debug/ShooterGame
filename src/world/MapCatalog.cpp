#include "world/MapCatalog.h"
#include "core/Config.h"

namespace {
MapDefinition makeArenaMap() {
    const Material crate{Surface::Wood, {0.85f, 0.53f, 0.31f}, config::kWoodShininess};
    const Material pillar{Surface::Metal, {0.36f, 0.61f, 0.61f}, config::kMetalShininess};
    const MapTheme theme{
        {config::kClearRed, config::kClearGreen, config::kClearBlue},
        {0.16f, 0.39f, 0.67f},
        {0.32f, 0.34f, 0.38f},
        {-0.52f, -1.0f, -0.38f},
        {0.73f, 0.67f, 0.57f},
        config::kFogStart,
        config::kFogEnd,
        config::kShadowExtent,
        {Surface::Floor, {0.73f, 0.69f, 0.60f}, config::kFloorShininess},
        {{
            {{-4.0f, 2.8f, 3.0f}, {1.0f, 0.52f, 0.23f}},
            {{4.0f, 3.4f, -1.0f}, {0.30f, 0.65f, 1.0f}},
            {{-3.0f, 3.2f, -8.0f}, {0.95f, 0.73f, 0.41f}},
        }}};
    return {
        MapId::Arena,
        "ARENA",
        "CLASSIC COURTYARD - DAY",
        0,
        {config::kArenaSize, config::kArenaSize},
        config::kWallHeight,
        {0.0f, 0.0f, config::kStartZ},
        config::kStartYaw,
        config::kStartPitch,
        theme,
        {Surface::Wall, {0.75f, 0.73f, 0.67f}, config::kWallShininess},
        {
            {{-3.0f, config::kLowCrateHeight / 2.0f, 2.0f}, {3.0f, config::kLowCrateHeight, 2.0f}, crate},
            {{3.0f, 1.5f, -2.0f}, {2.0f, 3.0f, 3.0f}, pillar},
            {{-5.0f, 2.0f, -7.0f}, {2.0f, 4.0f, 2.0f}, pillar},
            {{1.0f, 0.8f, -8.0f}, {4.0f, 1.6f, 2.0f}, crate},
        },
        {{0.0f, 0.9f, 2.0f}, {1.35f, 1.8f, 0.30f},
         {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}},
        {
            {-4.0f, 0.0f, -1.0f}, {4.0f, 0.0f, 1.0f}, {9.0f, 0.0f, -8.0f}, {-10.0f, 0.0f, 4.0f},
            {-8.0f, 0.0f, -11.0f}, {8.0f, 0.0f, 9.0f}, {1.0f, 0.0f, -12.0f}, {-12.0f, 0.0f, -4.0f},
            {12.0f, 0.0f, 3.0f}, {-7.0f, 0.0f, 11.0f}, {7.0f, 0.0f, -12.0f}, {0.0f, 0.0f, 12.0f},
        }};
}

MapDefinition makeWarehouseMap() {
    const Material blueContainer{Surface::Metal, {0.26f, 0.44f, 0.62f}, config::kMetalShininess};
    const Material redContainer{Surface::Metal, {0.70f, 0.30f, 0.24f}, config::kMetalShininess};
    const Material woodPallet{Surface::Wood, {0.78f, 0.55f, 0.32f}, config::kWoodShininess};
    const Material steelColumn{Surface::Metal, {0.42f, 0.45f, 0.49f}, config::kMetalShininess};
    const MapTheme theme{
        {0.86f, 0.52f, 0.32f},
        {0.20f, 0.17f, 0.38f},
        {0.30f, 0.24f, 0.24f},
        {-0.82f, -0.62f, -0.35f},
        {0.92f, 0.58f, 0.34f},
        11.0f,
        46.0f,
        25.0f,
        {Surface::Floor, {0.52f, 0.53f, 0.55f}, 26.0f},
        {{
            {{-9.0f, 4.2f, 2.0f}, {1.15f, 0.72f, 0.30f}},
            {{9.0f, 4.2f, -2.0f}, {1.15f, 0.68f, 0.28f}},
            {{0.0f, 4.4f, -6.5f}, {0.42f, 0.78f, 1.05f}},
        }}};
    return {
        MapId::Warehouse,
        "WAREHOUSE",
        "SHIPPING DEPOT - SUNSET",
        0,
        {34.0f, 26.0f},
        5.0f,
        {0.0f, 0.0f, 10.0f},
        -90.0f,
        -4.0f,
        theme,
        {Surface::Wall, {0.58f, 0.53f, 0.48f}, config::kWallShininess},
        {
            {{-10.0f, 1.35f, 3.0f}, {2.8f, 2.7f, 7.0f}, blueContainer},
            {{-9.5f, 1.35f, -6.5f}, {5.6f, 2.7f, 2.8f}, redContainer},
            {{10.0f, 1.35f, 3.0f}, {2.8f, 2.7f, 7.0f}, redContainer},
            {{9.5f, 1.35f, -6.5f}, {5.6f, 2.7f, 2.8f}, blueContainer},
            {{0.0f, config::kLowCrateHeight * 0.5f, -1.5f}, {5.2f, config::kLowCrateHeight, 3.2f}, woodPallet},
            {{0.0f, config::kLowCrateHeight + 0.325f, -2.1f}, {2.4f, 0.65f, 1.4f}, woodPallet},
            {{-4.8f, config::kLowCrateHeight * 0.5f, 6.2f}, {2.2f, config::kLowCrateHeight, 2.0f}, woodPallet},
            {{5.0f, 0.85f, 6.0f}, {2.2f, 1.70f, 2.0f}, woodPallet},
            {{0.0f, 2.2f, -8.8f}, {2.2f, 4.4f, 2.0f}, steelColumn},
        },
        {{0.0f, 0.9f, 3.8f}, {1.35f, 1.8f, 0.30f},
         {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}},
        {
            {-14.5f, 0.0f, 9.0f}, {14.5f, 0.0f, 9.0f}, {-14.5f, 0.0f, -2.5f}, {14.5f, 0.0f, -2.5f},
            {-14.5f, 0.0f, -10.5f}, {14.5f, 0.0f, -10.5f}, {-5.0f, 0.0f, -10.8f}, {5.0f, 0.0f, -10.8f},
            {-5.5f, 0.0f, -2.2f}, {5.5f, 0.0f, -2.2f}, {-6.0f, 0.0f, 10.5f}, {6.0f, 0.0f, 10.5f},
        }};
}

MapDefinition makeCitadelMap() {
    const Material stonePillar{Surface::Wall, {0.50f, 0.54f, 0.64f}, 24.0f};
    const Material obeliskMetal{Surface::Metal, {0.30f, 0.52f, 0.68f}, 96.0f};
    const Material darkWood{Surface::Wood, {0.52f, 0.36f, 0.26f}, config::kWoodShininess};
    const MapTheme theme{
        {0.10f, 0.15f, 0.26f},
        {0.02f, 0.04f, 0.11f},
        {0.16f, 0.19f, 0.27f},
        {0.42f, -1.0f, 0.45f},
        {0.34f, 0.43f, 0.62f},
        8.0f,
        36.0f,
        25.0f,
        {Surface::Floor, {0.42f, 0.47f, 0.54f}, 38.0f},
        {{
            {{0.0f, 3.6f, 4.5f}, {0.35f, 0.92f, 1.35f}},
            {{-8.5f, 3.2f, -4.5f}, {1.40f, 0.62f, 0.22f}},
            {{8.5f, 3.2f, -4.5f}, {1.40f, 0.62f, 0.22f}},
        }}};
    return {
        MapId::Citadel,
        "CITADEL",
        "FORTRESS KEEP - NIGHT",
        0,
        {32.0f, 32.0f},
        5.5f,
        {0.0f, 0.0f, 12.5f},
        -90.0f,
        -4.0f,
        theme,
        {Surface::Wall, {0.44f, 0.48f, 0.56f}, 18.0f},
        {
            {{0.0f, 2.4f, -1.0f}, {2.8f, 4.8f, 2.8f}, obeliskMetal},
            {{-8.0f, 2.6f, -8.0f}, {2.6f, 5.2f, 2.6f}, stonePillar},
            {{8.0f, 2.6f, -8.0f}, {2.6f, 5.2f, 2.6f}, stonePillar},
            {{-8.0f, 2.6f, 6.0f}, {2.6f, 5.2f, 2.6f}, stonePillar},
            {{8.0f, 2.6f, 6.0f}, {2.6f, 5.2f, 2.6f}, stonePillar},
            {{-8.0f, 0.85f, -1.0f}, {1.6f, 1.7f, 4.2f}, stonePillar},
            {{8.0f, 0.85f, -1.0f}, {1.6f, 1.7f, 4.2f}, stonePillar},
            {{0.0f, config::kLowCrateHeight * 0.5f, -9.5f}, {4.4f, config::kLowCrateHeight, 2.0f}, darkWood},
            {{-3.2f, config::kLowCrateHeight * 0.5f, 6.0f}, {2.0f, config::kLowCrateHeight, 2.0f}, darkWood},
            {{3.2f, config::kLowCrateHeight * 0.5f, 6.0f}, {2.0f, config::kLowCrateHeight, 2.0f}, darkWood},
        },
        {{0.0f, 0.9f, 4.2f}, {1.35f, 1.8f, 0.30f},
         {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}},
        {
            {-13.0f, 0.0f, 11.0f}, {13.0f, 0.0f, 11.0f}, {-13.0f, 0.0f, -1.0f}, {13.0f, 0.0f, -1.0f},
            {-13.0f, 0.0f, -12.5f}, {13.0f, 0.0f, -12.5f}, {0.0f, 0.0f, -13.5f}, {-4.2f, 0.0f, -4.5f},
            {4.2f, 0.0f, -4.5f}, {-4.5f, 0.0f, 2.5f}, {4.5f, 0.0f, 2.5f}, {-6.0f, 0.0f, 12.5f},
        }};
}

MapDefinition makeLabyrinthMap() {
    const Material ruinWall{Surface::Wall, {0.78f, 0.62f, 0.42f}, 12.0f};
    const Material carvedStone{Surface::Wall, {0.66f, 0.56f, 0.44f}, 16.0f};
    const Material crateWood{Surface::Wood, {0.72f, 0.48f, 0.26f}, config::kWoodShininess};
    const Material brassPillar{Surface::Metal, {0.76f, 0.62f, 0.34f}, 68.0f};
    const MapTheme theme{
        {0.88f, 0.81f, 0.66f},
        {0.24f, 0.56f, 0.84f},
        {0.38f, 0.35f, 0.29f},
        {0.30f, -1.0f, -0.24f},
        {0.92f, 0.86f, 0.72f},
        12.0f,
        44.0f,
        24.0f,
        {Surface::Floor, {0.84f, 0.74f, 0.54f}, 10.0f},
        {{
            {{0.0f, 3.0f, 0.0f}, {0.30f, 0.82f, 0.76f}},
            {{-8.5f, 2.8f, -8.5f}, {1.05f, 0.72f, 0.36f}},
            {{8.5f, 2.8f, 8.5f}, {1.05f, 0.64f, 0.32f}},
        }}};
    return {
        MapId::Labyrinth,
        "LABYRINTH",
        "SANDSTONE RUINS - NOON",
        0,
        {30.0f, 30.0f},
        3.8f,
        {-11.5f, 0.0f, 11.5f},
        -45.0f,
        -4.0f,
        theme,
        {Surface::Wall, {0.82f, 0.67f, 0.46f}, 10.0f},
        {
            {{-7.5f, 1.5f, -5.0f}, {7.0f, 3.0f, 1.4f}, ruinWall},
            {{7.5f, 1.5f, -5.0f}, {7.0f, 3.0f, 1.4f}, ruinWall},
            {{-7.5f, 1.5f, 5.0f}, {7.0f, 3.0f, 1.4f}, ruinWall},
            {{7.5f, 1.5f, 5.0f}, {7.0f, 3.0f, 1.4f}, ruinWall},
            {{-5.0f, 1.3f, 0.0f}, {1.4f, 2.6f, 3.2f}, carvedStone},
            {{5.0f, 1.3f, 0.0f}, {1.4f, 2.6f, 3.2f}, carvedStone},
            {{0.0f, 1.6f, -10.2f}, {1.6f, 3.2f, 4.0f}, carvedStone},
            {{0.0f, 1.6f, 10.2f}, {1.6f, 3.2f, 4.0f}, carvedStone},
            {{0.0f, config::kLowCrateHeight * 0.5f, 0.0f}, {2.6f, config::kLowCrateHeight, 2.6f}, brassPillar},
            {{-4.5f, config::kLowCrateHeight * 0.5f, 10.5f}, {2.2f, config::kLowCrateHeight, 1.8f}, crateWood},
            {{10.0f, 1.9f, -10.0f}, {2.0f, 3.8f, 2.0f}, brassPillar},
        },
        {{-8.5f, 0.9f, 8.5f}, {1.35f, 1.8f, 0.30f},
         {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}},
        {
            {-12.5f, 0.0f, 0.0f}, {12.5f, 0.0f, 0.0f}, {-12.5f, 0.0f, -10.5f}, {13.0f, 0.0f, 11.5f},
            {5.5f, 0.0f, -12.5f}, {-5.5f, 0.0f, -12.5f}, {5.5f, 0.0f, 12.5f}, {0.0f, 0.0f, -4.5f},
            {0.0f, 0.0f, 4.5f}, {-9.0f, 0.0f, 0.0f}, {9.0f, 0.0f, 0.0f}, {5.5f, 0.0f, -8.5f},
        }};
}

const std::array<MapDefinition, kMapCount>& catalog() {
    static const std::array<MapDefinition, kMapCount> kMaps{{
        makeArenaMap(),
        makeWarehouseMap(),
        makeCitadelMap(),
        makeLabyrinthMap(),
    }};
    return kMaps;
}
}

MapId mapIdFromIndex(std::size_t index) {
    return catalog()[index % kMapCount].id;
}

std::size_t mapIndex(MapId id) {
    const auto raw = static_cast<std::size_t>(id);
    return raw < kMapCount ? raw : 0;
}

const MapDefinition& mapDefinition(MapId id) {
    return catalog()[mapIndex(id)];
}

const MapDefinition& mapDefinition(std::size_t index) {
    return catalog()[index % kMapCount];
}
