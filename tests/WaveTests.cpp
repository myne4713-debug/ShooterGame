#include "world/WaveManager.h"
#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace {
constexpr float kDt = static_cast<float>(config::kPhysicsStep);

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void testRoutes(const Level& level) {
    ArenaRoutes routes(level.solids());
    for (auto goal : {glm::vec3{0,config::kEyeHeight,9}, glm::vec3{4,config::kEyeHeight,-6},
                      glm::vec3{-8,config::kEyeHeight,-12}, glm::vec3{13.65f,config::kEyeHeight,13.65f}}) {
        routes.setGoal(goal);
        for (glm::vec3 start : {glm::vec3{0,0,-12}, glm::vec3{3,0,4}, glm::vec3{-12,0,-10}}) {
            Enemy enemy(EnemyType::Guard, start);
            require(enemy.state() == EnemyState::Idle, "initial idle");
            bool attacked = false;
            for (int i = 0; i < 2400 && !attacked; ++i) {
                attacked = enemy.step(kDt, goal, routes.waypoint(enemy.feet()), level.solids()) > 0;
                const auto body = enemy.bounds();
                for (const auto& box : level.solids()) {
                    const bool inside = glm::all(glm::greaterThan(body.max, box.min+.0001f)) &&
                                        glm::all(glm::lessThan(body.min, box.max-.0001f));
                    require(!inside, "crossed solid");
                }
            }
            require(attacked && enemy.state() == EnemyState::Attack, "could not reach player behind cover");
        }
    }
}

void testContact(const Level& level) {
    Enemy enemy(EnemyType::Guard, {0,0,1.2f});
    for (int i = 0; i < 120; ++i) {
        require(enemy.step(kDt, {0,1.65f,2.6f}, enemy.feet(), level.solids()) == 0,
                "practice board blocks contact damage");
    }
    const glm::vec3 eye{0,1.65f,.6f};
    require(enemy.step(kDt, eye, enemy.feet(), level.solids()) == config::kGuardDamage, "contact in range");
    require(enemy.step(kDt, eye, enemy.feet(), level.solids()) == 0, "attack cooldown");
    enemy.takeDamage(enemy.maxHealth());
    require(enemy.step(config::kAttackInterval, eye, enemy.feet(), level.solids()) == 0, "dead enemy cannot attack");
}

void testWaveLoop(Level& level) {
    WaveManager waves(level);
    const glm::vec3 eye{0,config::kEyeHeight,config::kStartZ};
    require(waves.enemies().front().type() != waves.enemies().back().type(), "enemy types");
    const auto blocked = waves.hitScan(level, {-4,.4f,5}, {0,0,-1}, 100);
    require(blocked && !blocked->movingTarget && waves.score() == 0, "crate blocks enemy");
    const glm::vec3 robot = waves.enemies().front().feet() + glm::vec3{0,1,0};
    const glm::vec3 towardBoard = glm::normalize(glm::vec3{0,1,2} - robot);
    const auto first = waves.hitScan(level, robot - towardBoard, towardBoard, config::kShotDamage);
    require(first && first->movingTarget && level.targetHealth() == config::kPracticeHealth,
            "enemy in front shields board");
    int expected = 0;
    for (int wave = 1; wave <= 9; ++wave) {
        require(waves.wave() == wave, "wave counter");
        require(static_cast<int>(waves.enemies().size()) == std::min(wave+1,config::kMaxWaveCount), "wave size");
        for (const auto& enemy : waves.enemies()) {
            const auto origin = enemy.feet() + glm::vec3{0,1,.6f};
            while (enemy.health() > 0) {
                const auto hit = waves.hitScan(level, origin, {0,0,-1}, config::kShotDamage);
                require(hit && hit->movingTarget && hit->damagedTarget, "missed living enemy");
            }
            expected += enemy.points();
            waves.hitScan(level, origin, {0,0,-1}, config::kShotDamage);
            require(waves.score() == expected, "kill must score once");
        }
        require(!waves.step(kDt, eye, level) && waves.enemies().front().deathScale() < 1, "death animation");
        bool spawned = false;
        for (int i = 0; i < 150 && !spawned; ++i) spawned = waves.step(kDt, eye, level);
        require(spawned && waves.health() == config::kPlayerHealth, "advance after clearing wave");
    }
    for (int i = 0; i < 3600 && waves.health() > 0; ++i) waves.step(kDt, eye, level);
    require(waves.health() == 0 && waves.damageFraction() > 0, "game over");
    WaveManager fresh(level);
    require(fresh.wave() == 1 && fresh.score() == 0 && fresh.health() == config::kPlayerHealth, "fresh round");
}
}

int main() {
    try {
        Level level;
        testRoutes(level);
        testContact(level);
        testWaveLoop(level);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Wave test failed: %s\n", error.what());
        return 1;
    }
    return 0;
}
