#include "physics/AABB.h"
#include "world/Level.h"
#include "world/Player.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
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
}

int main() {
    try {
        testRay();
        testSweep();
        testCrate();
        testCeiling();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Physics test failed: %s\n", error.what());
        return 1;
    }
    return 0;
}
