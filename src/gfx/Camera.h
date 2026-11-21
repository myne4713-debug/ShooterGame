#pragma once

#include "core/Config.h"
#include <glm/glm.hpp>

class Camera {
public:
    Camera() = default;
    Camera(glm::vec3 position, float yaw, float pitch = config::kStartPitch)
        : m_position(position), m_yaw(yaw), m_pitch(pitch) {}
    void look(glm::vec2 mouseDelta);
    void applyRecoil(float degrees);
    glm::vec3 groundDirection(glm::vec2 strafeForward) const;
    glm::vec3 front() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjection(float aspect) const;
    glm::vec3 position() const { return m_position; }
    void setPosition(glm::vec3 position) { m_position = position; }

private:
    glm::vec3 m_position{0, config::kEyeHeight, config::kStartZ};
    float m_yaw = config::kStartYaw;
    float m_pitch = config::kStartPitch;
    glm::vec3 groundForward() const;
};
