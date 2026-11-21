#include "physics/AABB.h"
#include "world/ImageMapLoader.h"
#include "world/Level.h"
#include "world/MapCatalog.h"
#include "world/Player.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(float a, float b) {
    return std::abs(a - b) < 0.0001f;
}

void testRay() {
    const AABB box{{-1, -1, -1}, {1, 1, 1}};
    require(rayVsAABB({3, 0, 0}, {-2, 0, 0}, box) == 1.0f, "ray with negative direction");
    require(rayVsAABB({0, 0, 0}, {1, 0, 0}, box) == 0.0f, "origin inside");
    require(rayVsAABB({-3, 1, 1}, {1, 0, 0}, box) == 2.0f, "ray on edge");
    require(!rayVsAABB({2, 0, 0}, {0, 0, 1}, box), "parallel outside slab");
    require(!rayVsAABB({2, 0, 0}, {1, 0, 0}, box), "box behind ray");
    require(!rayVsAABB({-3, 0, 0}, {1, 2, 0}, box), "disjoint slab intervals");
    require(!rayVsAABB({0, 0, 0}, {0, 0, 0}, box), "zero direction");
}

void testSweep() {
    const AABB body{{-0.25f, 0, -0.25f}, {0.25f, 1, 0.25f}};
    const std::vector<AABB> walls{
        {{8, 0, -10}, {9, 3, 10}}, {{3, 0, -10}, {3.02f, 3, 10}},
        {{-10, 0, 4}, {10, 3, 5}}
    };
    glm::vec3 feet{0, 0, 0};
    require(sweepAxis(feet, body, 0, 100.0f, walls) && near(feet.x, 2.75f), "nearest thin wall");
    require(!sweepAxis(feet, body, 2, 1.0f, walls) && near(feet.z, 1.0f), "slide along wall");
    require(sweepAxis(feet, body, 2, 100.0f, walls) && near(feet.z, 3.75f), "closed corner");
    require(!sweepAxis(feet, body, 0, -1.0f, walls), "leave contact");
    feet = {0, 3, 0};
    require(!sweepAxis(feet, body, 0, 5.0f, walls), "walk across top face");
}

void testCrate() {
    const Level level;
    Player player;
    const float speed = config::kWalkSpeed;
    const auto advance = [&](int count, glm::vec3 velocity) {
        for (int i = 0; i < count; ++i) {
            player.step(velocity, false, level.solids());
        }
    };
    advance(45, {-speed, 0, 0});
    advance(120, {0, 0, -speed});
    require(near(player.eyePosition().z, 3.0f + config::kBodyHalfWidth), "crate side blocks walking");
    player.step({0, 0, -speed}, true, level.solids());
    advance(23, {0, 0, -speed});
    require(!player.grounded(), "jump clears edge");
    advance(60, {0, 0, 0});
    const float topEye = config::kEyeHeight + config::kLowCrateHeight;
    require(player.grounded() && near(player.eyePosition().y, topEye), "land on visible crate");
    player.step({0, 0, 0}, true, level.solids());
    float peak = player.eyePosition().y;
    for (int i = 0; i < 70; ++i) {
        player.step({0, 0, 0}, i == 10, level.solids());
        peak = std::max(peak, player.eyePosition().y);
    }
    const float jumpHeight = config::kJumpSpeed * config::kJumpSpeed / (2.0f * config::kGravity);
    require(near(peak, topEye + jumpHeight), "jump from crate; second press in air ignored");
    require(player.grounded() && near(player.eyePosition().y, topEye), "stable landing on crate");
    advance(45, {speed, 0, 0});
    require(!player.grounded() && player.eyePosition().y < topEye, "fall after leaving edge");
    advance(60, {0, 0, 0});
    require(player.grounded() && near(player.eyePosition().y, config::kEyeHeight), "back on floor");
}

void testCeiling() {
    const float ceiling = config::kBodyHeight + 0.30f;
    const std::vector<AABB> solids{
        {{-20, 0, -20}, {20, 0, 20}}, {{-2, ceiling, 7}, {2, ceiling + 0.1f, 11}}
    };
    Player player;
    const float maxEye = ceiling - config::kBodyHeight + config::kEyeHeight;
    bool touched = false;
    for (int i = 0; i < 60; ++i) {
        player.step({0, 0, 0}, i == 0, solids);
        require(player.eyePosition().y <= maxEye + 0.0001f, "head passed ceiling");
        if (near(player.eyePosition().y, maxEye)) {
            require(!player.grounded(), "ceiling is not support");
            touched = true;
        }
    }
    require(touched && player.grounded() && near(player.eyePosition().y, config::kEyeHeight),
            "hit ceiling and return to floor");
}

bool overlapsAnySolid(const AABB& body, const std::vector<AABB>& solids) {
    for (const auto& box : solids) {
        if (glm::all(glm::greaterThan(body.max, box.min + 0.0001f))
            && glm::all(glm::lessThan(body.min, box.max - 0.0001f))) {
            return true;
        }
    }
    return false;
}

void verifyMapPlacement(const MapDefinition& def) {
    require(!def.name.empty() && !def.subtitle.empty(), "map labels present");
    require(def.arenaSize.x > 0 && def.arenaSize.y > 0 && def.wallHeight > 0, "valid map dimensions");
    require(def.spawnPoints.size() >= config::kMaxWaveCount, "enough spawn points");

    const Level level(def);
    const float playerHalfX = level.arenaSize().x * 0.5f - config::kBodyHalfWidth;
    const float playerHalfZ = level.arenaSize().y * 0.5f - config::kBodyHalfWidth;
    const glm::vec3 spawn = level.playerSpawn();
    require(std::abs(spawn.x) <= playerHalfX && std::abs(spawn.z) <= playerHalfZ,
            "player spawn inside arena");
    const AABB playerBody{
        spawn + glm::vec3{-config::kBodyHalfWidth, 0.0f, -config::kBodyHalfWidth},
        spawn + glm::vec3{config::kBodyHalfWidth, config::kBodyHeight, config::kBodyHalfWidth}};
    require(!overlapsAnySolid(playerBody, level.solids()), "player spawn clear of solids");

    const float enemyHalfX = level.arenaSize().x * 0.5f - config::kEnemyRadius;
    const float enemyHalfZ = level.arenaSize().y * 0.5f - config::kEnemyRadius;
    for (const glm::vec3 pt : level.spawnPoints()) {
        require(std::abs(pt.x) <= enemyHalfX && std::abs(pt.z) <= enemyHalfZ,
                "enemy spawn inside arena");
        const AABB enemyBody{
            pt + glm::vec3{-config::kEnemyRadius, 0.0f, -config::kEnemyRadius},
            pt + glm::vec3{config::kEnemyRadius, config::kGuardHeight, config::kEnemyRadius}};
        require(!overlapsAnySolid(enemyBody, level.solids()), "enemy spawn clear of solids");
    }
}

void testAllMapsGeometry() {
    for (std::size_t i = 0; i < kMapCount; ++i) {
        verifyMapPlacement(mapDefinition(i));
    }

    // Kiểm tra nhảy 2 bậc lên bệ trung tâm trong bản đồ Warehouse.
    const Level warehouse(MapId::Warehouse);
    Player player(warehouse.playerSpawn());
    const float speed = config::kWalkSpeed;
    const auto advance = [&](int count, glm::vec3 velocity) {
        for (int i = 0; i < count; ++i) {
            player.step(velocity, false, warehouse.solids());
        }
    };
    advance(30, {speed, 0, 0});
    advance(135, {0, 0, -speed});
    advance(30, {-speed, 0, 0});
    advance(12, {0, 0, -speed});
    player.step({0, 0, -speed}, true, warehouse.solids());
    advance(15, {0, 0, -speed});
    advance(45, {0, 0, 0});
    const float tier1Eye = config::kEyeHeight + config::kLowCrateHeight;
    require(player.grounded() && near(player.eyePosition().y, tier1Eye), "land on warehouse tier 1");
    advance(15, {0, 0, -speed});
    player.step({0, 0, -speed}, true, warehouse.solids());
    advance(16, {0, 0, -speed});
    advance(45, {0, 0, 0});
    const float tier2Eye = tier1Eye + 0.65f;
    require(player.grounded() && near(player.eyePosition().y, tier2Eye), "land on warehouse tier 2");
}

std::filesystem::path sampleImagePath(const char* fileName) {
    const auto local = std::filesystem::path("assets") / "maps" / fileName;
    if (std::filesystem::exists(local)) return local;
    return std::filesystem::path("..") / "assets" / "maps" / fileName;
}

void testImageMapBuilder() {
    const auto checkResult = [](const ImageMapResult& result) {
        require(result.definition.id == MapId::Custom, "custom map id");
        require(result.definition.interiorBoxes.size() <= 28, "bounded interior box count");
        verifyMapPlacement(result.definition);
    };

    // 1. Sơ đồ tối trên nền sáng 128x128.
    std::vector<unsigned char> lightBg(128 * 128 * 4, 235);
    for (int y = 24; y < 104; ++y) {
        for (int x = 24; x < 104; ++x) {
            if ((y < 40 || x < 40 || x >= 88) && !(x >= 56 && x < 72)) {
                const auto idx = static_cast<std::size_t>(y * 128 + x) * 4;
                lightBg[idx] = 40;
                lightBg[idx + 1] = 50;
                lightBg[idx + 2] = 65;
            }
        }
    }
    checkResult(buildMapFromPixels(128, 128, lightBg.data(), "light_sketch", 1));

    // 2. Bản vẽ sáng trên nền tối 128x128.
    std::vector<unsigned char> darkBg(128 * 128 * 4, 24);
    for (int y = 20; y < 108; ++y) {
        for (int x = 20; x < 108; ++x) {
            if ((y >= 48 && y < 64 && (x < 52 || x >= 76)) || (x >= 28 && x < 44 && y < 44)) {
                const auto idx = static_cast<std::size_t>(y * 128 + x) * 4;
                darkBg[idx] = 80;
                darkBg[idx + 1] = 220;
                darkBg[idx + 2] = 240;
            }
        }
    }
    checkResult(buildMapFromPixels(128, 128, darkBg.data(), "dark_blueprint", 2));

    // 3. Ảnh màu gradient tỉ lệ 16:9.
    std::vector<unsigned char> gradient(160 * 90 * 4, 255);
    for (int y = 0; y < 90; ++y) {
        for (int x = 0; x < 160; ++x) {
            const auto idx = static_cast<std::size_t>(y * 160 + x) * 4;
            gradient[idx] = static_cast<unsigned char>((x * 255) / 159);
            gradient[idx + 1] = static_cast<unsigned char>((y * 255) / 89);
            gradient[idx + 2] = 128;
        }
    }
    const auto wide = buildMapFromPixels(160, 90, gradient.data(), "wide_sunset", 3);
    require(wide.gridWidth > wide.gridHeight, "aspect ratio preserved");
    checkResult(wide);

    // 4. Ảnh đồng màu trắng hoàn toàn.
    std::vector<unsigned char> flat(64 * 64 * 4, 255);
    checkResult(buildMapFromPixels(64, 64, flat.data(), "flat_white", 4));

    // 5. Ảnh bàn cờ nhiễu 16x16.
    std::vector<unsigned char> checker(16 * 16 * 4, 255);
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            const unsigned char v = ((x + y) % 2 == 0) ? 20 : 240;
            const auto idx = static_cast<std::size_t>(y * 16 + x) * 4;
            checker[idx] = checker[idx + 1] = checker[idx + 2] = v;
        }
    }
    checkResult(buildMapFromPixels(16, 16, checker.data(), "checker_noise", 5));

    // 6. Hai ảnh PNG mẫu kèm theo dự án.
    checkResult(loadMapFromImageFile(sampleImagePath("sample_maze.png"), 6));
    checkResult(loadMapFromImageFile(sampleImagePath("sample_neon_fort.png"), 7));
}
}

int main() {
    try {
        testRay();
        testSweep();
        testCrate();
        testCeiling();
        testAllMapsGeometry();
        testImageMapBuilder();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Physics test failed: %s\n", error.what());
        return 1;
    }
    return 0;
}
