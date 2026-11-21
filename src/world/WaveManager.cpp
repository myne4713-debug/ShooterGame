#include "world/WaveManager.h"
#include <algorithm>

WaveManager::WaveManager(const Level& level)
    : m_routes(level.solids(), level.arenaSize()), m_spawnPoints(level.spawnPoints()) {
    spawn(level.playerSpawn());
}

void WaveManager::spawn(glm::vec3 player) {
    ++m_wave;
    m_delay = config::kWaveDelay;
    m_enemies.clear();
    player.y = 0;
    const int count = std::min(config::kFirstWaveCount + m_wave - 1, config::kMaxWaveCount);
    const auto spawnCount = m_spawnPoints.size();
    for (std::size_t i = 0; i < spawnCount; ++i) {
        const auto feet = m_spawnPoints[(static_cast<std::size_t>(m_wave - 1) + i) % spawnCount];
        if (glm::distance(feet, player) < config::kMinSpawnDistance) continue;
        const bool guard = m_enemies.size() % 2 == 1;
        m_enemies.emplace_back(guard ? EnemyType::Guard : EnemyType::Runner, feet);
        if (static_cast<int>(m_enemies.size()) == count) break;
    }
}

bool WaveManager::step(float dt, glm::vec3 eye, const Level& level) {
    if (m_health == 0) return false;
    m_routes.setGoal(eye);
    m_flash = std::max(0.0f, m_flash - dt);
    bool alive = false;
    for (auto& enemy : m_enemies) {
        const bool dead = enemy.state() == EnemyState::Dead;
        const auto waypoint = dead ? enemy.feet() : m_routes.waypoint(enemy.feet());
        const int damage = enemy.step(dt, eye, waypoint, level.solids());
        if (damage > 0) {
            m_health = std::max(0, m_health - damage);
            m_flash = config::kDamageFlashSeconds;
        }
        alive |= !dead;
    }
    if (!alive && m_health > 0) {
        m_delay -= dt;
        if (m_delay <= 0) {
            spawn(eye);
            return true;
        }
    }
    return false;
}

std::optional<ShotHit> WaveManager::hitScan(Level& level, glm::vec3 origin, glm::vec3 direction, int damage) {
    Enemy* target = nullptr;
    float distance = config::kShotRange;
    glm::vec3 normal{};
    for (auto& enemy : m_enemies) {
        if (enemy.state() == EnemyState::Dead) continue;
        glm::vec3 face;
        const auto t = rayVsAABB(origin, direction, enemy.bounds(), &face);
        if (t && *t < distance) {
            target = &enemy;
            distance = *t;
            normal = face;
        }
    }
    // Giới hạn tia tĩnh ở mặt gần nhất của địch để bia tập phía sau cũng không nhận sát thương.
    auto wall = level.hitScan(origin, direction, damage, distance);
    if (wall || !target) return wall;
    target->takeDamage(damage);
    if (target->state() == EnemyState::Dead) m_score += target->points();
    return ShotHit{origin + distance * direction, normal, target->bounds(), true, true};
}
