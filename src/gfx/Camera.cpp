#include "gfx/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

void Camera::look(glm::vec2 mouseDelta) {
    // Delta chuột là quãng đường đã đi giữa hai lần đọc. Nhân thêm dt sẽ làm độ nhạy đổi theo FPS.
    m_yaw = std::remainder(m_yaw + mouseDelta.x * config::kMouseDegreesPerPixel, 360.0f);
    m_pitch = std::clamp(m_pitch - mouseDelta.y * config::kMouseDegreesPerPixel,
                         -config::kPitchLimit, config::kPitchLimit);
}

void Camera::applyRecoil(float degrees) {
    m_pitch = std::clamp(m_pitch + degrees, -config::kPitchLimit, config::kPitchLimit);
}

glm::vec3 Camera::groundForward() const {
    const float yaw = glm::radians(m_yaw);
    return {std::cos(yaw), 0.0f, std::sin(yaw)};
}

glm::vec3 Camera::groundDirection(glm::vec2 strafeForward) const {
    // Chỉ dùng yaw để khi ngẩng lên vẫn bước trên sàn với cùng vận tốc.
    const glm::vec3 forward = groundForward();
    const glm::vec3 right = glm::cross(forward, glm::vec3{0, 1, 0});
    strafeForward /= std::max(1.0f, glm::length(strafeForward));
    return right * strafeForward.x + forward * strafeForward.y;
}

glm::vec3 Camera::front() const {
    const float pitch = glm::radians(m_pitch);
    glm::vec3 direction = groundForward() * std::cos(pitch);
    direction.y = std::sin(pitch);
    return direction;
}

glm::mat4 Camera::getViewMatrix() const {
    // Kẹp pitch giữ front khác phương với trục đứng để lookAt luôn dựng được hệ right/up.
    return glm::lookAt(m_position, m_position + front(), glm::vec3{0, 1, 0});
}

glm::mat4 Camera::getProjection(float aspect) const {
    return glm::perspective(glm::radians(config::kVerticalFovDegrees), aspect,
                            config::kNearPlane, config::kFarPlane);
}
