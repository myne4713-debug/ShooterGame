#include "world/Player.h"

void Player::step(glm::vec3 horizontalVelocity, bool jumpPressed, const std::vector<AABB>& solids) {
    constexpr float dt = static_cast<float>(config::kPhysicsStep);
    constexpr float r = config::kBodyHalfWidth;
    const AABB body{{-r, 0, -r}, {r, config::kBodyHeight, r}};

    // X và Z giải riêng: một cạnh tường chặn hướng đi vào, vẫn cho phép đi dọc cạnh.
    sweepAxis(m_feet, body, 0, horizontalVelocity.x * dt, solids);
    sweepAxis(m_feet, body, 2, horizontalVelocity.z * dt, solids);
    if (jumpPressed && m_grounded) {
        m_verticalSpeed = config::kJumpSpeed;
    }
    const float dy = m_verticalSpeed * dt - 0.5f * config::kGravity * dt * dt;
    m_verticalSpeed -= config::kGravity * dt;
    const bool hit = sweepAxis(m_feet, body, 1, dy, solids);
    m_grounded = hit && dy < 0.0f;
    if (hit) {
        m_verticalSpeed = 0.0f;
    }
}

glm::vec3 Player::eyePosition() const {
    return m_feet + glm::vec3{0, config::kEyeHeight, 0};
}
