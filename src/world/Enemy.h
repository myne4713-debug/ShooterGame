#pragma once

#include "physics/AABB.h"

enum class EnemyType { Runner, Guard };
enum class EnemyState { Idle, Chase, Attack, Dead };

class Enemy {
public:
    Enemy(EnemyType type, glm::vec3 feet);
    int step(float dt, glm::vec3 eye, glm::vec3 waypoint, const std::vector<AABB>& solids);
    void takeDamage(int damage);
    AABB bounds() const;
    int maxHealth() const;
    float height() const;
    int points() const;
    EnemyType type() const { return m_type; }
    EnemyState state() const { return m_state; }
    glm::vec3 feet() const { return m_feet; }
    float yaw() const { return m_yaw; }
    float phase() const { return m_phase; }
    int health() const { return m_health; }
    float deathScale() const;

private:
    EnemyType m_type;
    EnemyState m_state = EnemyState::Idle;
    glm::vec3 m_feet;
    int m_health;
    float m_yaw = 0, m_phase = 0, m_timer = 0;
};
