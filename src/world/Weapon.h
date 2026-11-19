#pragma once

#include "core/Config.h"
#include <glm/vec3.hpp>
#include <random>

class Weapon {
public:
    // Trả true chỉ ở bước mô phỏng đã tiêu thụ một viên.
    bool step(double dt, bool trigger, bool reload);
    glm::vec3 shotDirection(glm::vec3 front);
    int magazine() const { return m_magazine; }
    int reserve() const { return m_reserve; }
    int damage() const { return config::kShotDamage; }
    bool reloading() const { return m_reloadLeft > 0.0; }
    void refillReserve() { m_reserve = config::kStartingReserve; }
    float reloadFraction() const;

private:
    int m_magazine = config::kMagazineSize;
    int m_reserve = config::kStartingReserve;
    double m_cooldown = 0.0;
    double m_reloadLeft = 0.0;
    std::mt19937 m_random{std::random_device{}()};
};
