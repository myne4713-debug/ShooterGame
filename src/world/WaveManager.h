#pragma once
#include "world/ArenaRoutes.h"
#include "world/Enemy.h"
#include "world/Level.h"

class WaveManager {
public:
    explicit WaveManager(const Level& level);
    // Trả true khi bắt đầu đợt kế tiếp để cấp lại đạn dự trữ.
    bool step(float dt, glm::vec3 eye, const Level& level);
    std::optional<ShotHit> hitScan(Level& level, glm::vec3 origin, glm::vec3 direction, int damage);
    const std::vector<Enemy>& enemies() const { return m_enemies; }
    int health() const { return m_health; }
    int wave() const { return m_wave; }
    int score() const { return m_score; }
    float damageFraction() const { return m_flash / config::kDamageFlashSeconds; }

private:
    void spawn(glm::vec3 player);
    ArenaRoutes m_routes;
    std::vector<glm::vec3> m_spawnPoints;
    std::vector<Enemy> m_enemies;
    int m_wave = 0, m_score = 0, m_health = config::kPlayerHealth;
    float m_delay = 0, m_flash = 0;
};
