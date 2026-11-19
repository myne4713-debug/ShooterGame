#include "world/Enemy.h"
#include "core/Config.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

Enemy::Enemy(EnemyType type, glm::vec3 feet)
    : m_type(type), m_feet(feet), m_health(maxHealth()), m_timer(config::kEnemyWakeSeconds) {}

int Enemy::maxHealth() const {
    return m_type == EnemyType::Runner ? config::kRunnerHealth : config::kGuardHealth;
}

float Enemy::height() const {
    return m_type == EnemyType::Runner ? config::kRunnerHeight : config::kGuardHeight;
}

int Enemy::points() const {
    return m_type == EnemyType::Runner ? config::kRunnerPoints : config::kGuardPoints;
}

AABB Enemy::bounds() const {
    constexpr float r = config::kEnemyRadius;
    return {m_feet + glm::vec3{-r, 0, -r}, m_feet + glm::vec3{r, height(), r}};
}

void Enemy::takeDamage(int damage) {
    if (m_state == EnemyState::Dead) return;
    m_health = std::max(0, m_health - damage);
    if (m_health == 0) {
        m_state = EnemyState::Dead;
        m_timer = config::kDeathSeconds;
    }
}

float Enemy::deathScale() const {
    return m_state == EnemyState::Dead ? m_timer / config::kDeathSeconds : 1.0f;
}

int Enemy::step(float dt, glm::vec3 eye, glm::vec3 waypoint, const std::vector<AABB>& solids) {
    m_timer = std::max(0.0f, m_timer - dt);
    if (m_state == EnemyState::Dead || (m_state == EnemyState::Idle && m_timer > 0)) return 0;

    const glm::vec3 target = eye - m_feet;
    // Mũi mô hình hướng theo +Z. atan2(x,z) giữ đúng góc quay ở cả bốn góc phần tư.
    if (target.x != 0 || target.z != 0) m_yaw = std::atan2(target.x, target.z);
    const glm::vec3 chest = m_feet + glm::vec3{0, height() * 0.75f, 0};
    const glm::vec3 toEye = eye - chest;
    bool canAttack = glm::length(toEye) <= config::kAttackRange;
    if (canAttack) {
        for (const AABB& box : solids) {
            const auto t = rayVsAABB(chest, toEye, box);
            if (t && *t < 1.0f) {
                canAttack = false;
                break;
            }
        }
    }
    if (canAttack) {
        m_state = EnemyState::Attack;
        if (m_timer > 0) return 0;
        m_timer = config::kAttackInterval;
        return m_type == EnemyType::Runner ? config::kRunnerDamage : config::kGuardDamage;
    }
    m_state = EnemyState::Chase;
    const glm::vec3 delta{waypoint.x - m_feet.x, 0, waypoint.z - m_feet.z};
    const float distance = glm::length(delta);
    if (distance == 0) return 0;
    const float speed = m_type == EnemyType::Runner ? config::kRunnerSpeed : config::kGuardSpeed;
    const glm::vec3 move = delta * (std::min(distance, speed * dt) / distance);
    const auto box = bounds();
    const AABB local{box.min - m_feet, box.max - m_feet};
    const auto before = m_feet;
    sweepAxis(m_feet, local, 0, move.x, solids);
    sweepAxis(m_feet, local, 2, move.z, solids);
    m_phase += glm::length(m_feet - before) * config::kBobFrequency;
    return 0;
}
