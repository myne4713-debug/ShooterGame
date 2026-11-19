#include "world/Level.h"
#include "world/Weapon.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

void testAmmo() {
    Weapon gun;
    require(!gun.step(0, false, true) && !gun.reloading(), "full magazine");
    require(gun.step(0, true, false), "first press");
    require(!gun.step(0.04, true, false), "cooldown");
    require(!gun.step(0, true, true) && gun.reloading(), "reload takes priority");
    require(!gun.step(0.5, true, true) && gun.magazine() == 11 && gun.reserve() == 42, "reload in progress");
    require(!gun.step(0.71, false, false) && !gun.reloading(), "reload complete");
    require(gun.magazine() == 12 && gun.reserve() == 41, "partial reload accounting");

    int remaining = gun.magazine() + gun.reserve();
    while (remaining > 0) {
        while (gun.magazine() > 0) {
            require(gun.step(config::kShotInterval, true, false), "loaded gun should fire");
            require(gun.magazine() + gun.reserve() == --remaining, "one round per shot");
        }
        require(!gun.step(config::kShotInterval, true, false), "empty magazine");
        gun.step(0, false, true);
        require(gun.reloading() == (remaining > 0), "reserve available");
        gun.step(config::kReloadSeconds, false, false);
        require(gun.magazine() == std::min(remaining, config::kMagazineSize), "last partial magazine");
    }
}

void testHits() {
    Level level;
    Weapon gun;
    const auto blocked = level.hitScan({0, 0.9f, -12}, {0, 0, 1}, gun.damage());
    require(blocked && !blocked->damagedTarget && blocked->normal.z == -1, "crate blocks target");
    require(level.targetHealth() == config::kPracticeHealth, "occluded target unchanged");
    const auto miss = level.hitScan({0, 3, 9}, {0, 1, 0}, gun.damage());
    require(!miss, "open sky");
    const auto floor = level.hitScan({0, 2, 9}, {0, -1, 0}, gun.damage());
    require(floor && floor->point.y == 0 && floor->normal.y == 1, "floor entry normal");
    const glm::vec3 eye{0, config::kEyeHeight, config::kStartZ};
    const glm::vec3 front = glm::normalize(glm::vec3{0, .9f, 2} - eye);
    for (int i = 0; i < 5; ++i) {
        const auto dir = gun.shotDirection(front);
        require(std::abs(glm::length(dir) - 1) < 0.00001f, "unit ray");
        require(glm::dot(dir, front) >= std::cos(glm::radians(config::kSpreadDegrees)) - 0.00001f, "spread cone");
        const auto hit = level.hitScan(eye, dir, gun.damage());
        require(hit && hit->normal.z == 1 && hit->damagedTarget == (i < 4), "target entry face and damage");
        require(level.targetHealth() == std::max(0, 100 - 25*(i+1)), "target health");
    }
}

void testHeldTrigger() {
    Weapon gun;
    // 0,15 giây ứng với 9 bước. Nhấn giữ trong 1,5 giây tạo đúng 10 phát.
    for (int tick = 0; tick < 90; ++tick) {
        require(gun.step(config::kPhysicsStep, true, false) == (tick % 9 == 0), "held trigger cadence");
    }
}
}

int main() {
    try {
        testAmmo();
        testHeldTrigger();
        testHits();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Combat test failed: %s\n", error.what());
        return 1;
    }
    return 0;
}
