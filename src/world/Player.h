#pragma once

#include "core/Config.h"
#include "physics/AABB.h"

class Player {
public:
    // Một bước vật lý cố định. Vận tốc ngang tính theo đơn vị thế giới/giây.
    void step(glm::vec3 horizontalVelocity, bool jumpPressed, const std::vector<AABB>& solids);
    glm::vec3 eyePosition() const;
    bool grounded() const { return m_grounded; }

private:
    glm::vec3 m_feet{0, 0, config::kStartZ};
    float m_verticalSpeed = 0.0f;
    bool m_grounded = true;
};
