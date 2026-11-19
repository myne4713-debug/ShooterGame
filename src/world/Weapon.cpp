#include "world/Weapon.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
double remainingTime(double time, double dt) {
    // Sau nhiều bước 1/60, sai số cộng dồn ở mốc 0 không được kéo dài thêm một bước.
    const double remaining = time - dt;
    return remaining > 16 * std::numeric_limits<double>::epsilon() ? remaining : 0.0;
}
}

bool Weapon::step(double dt, bool trigger, bool reload) {
    m_cooldown = remainingTime(m_cooldown, dt);
    if (reloading()) {
        m_reloadLeft = remainingTime(m_reloadLeft, dt);
        if (!reloading()) {
            const int count = std::min(config::kMagazineSize - m_magazine, m_reserve);
            m_magazine += count;
            m_reserve -= count;
        }
        return false;
    }
    if (reload && m_magazine < config::kMagazineSize && m_reserve > 0) {
        m_reloadLeft = config::kReloadSeconds;
        return false;
    }
    if (!trigger || m_cooldown > 0.0 || m_magazine == 0) {
        return false;
    }
    --m_magazine;
    m_cooldown = config::kShotInterval;
    return true;
}

float Weapon::reloadFraction() const {
    return static_cast<float>(1.0 - m_reloadLeft / config::kReloadSeconds);
}

glm::vec3 Weapon::shotDirection(glm::vec3 front) {
    const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3{0, 1, 0}));
    const glm::vec3 up = glm::cross(right, front);
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    const float angle = glm::two_pi<float>() * uniform(m_random);
    // Căn bậc hai cho mẫu đều trên diện tích đĩa nằm vuông góc với hướng nhìn.
    const float radius = std::tan(glm::radians(config::kSpreadDegrees)) * std::sqrt(uniform(m_random));
    return glm::normalize(front + radius * (std::cos(angle) * right + std::sin(angle) * up));
}
