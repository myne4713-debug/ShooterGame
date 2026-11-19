#include "gfx/WeaponRenderer.h"
#include "gfx/Camera.h"
#include "gfx/Primitives.h"
#include "world/Weapon.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

namespace {
// Các cột là ba trục cục bộ và vị trí. Trục phụ khác phương Z để tích có hướng luôn xác định.
glm::mat4 frame(glm::vec3 position, glm::vec3 z) {
    const glm::vec3 ref = std::abs(z.y) < 0.99f ? glm::vec3{0, 1, 0} : glm::vec3{1, 0, 0};
    const glm::vec3 x = glm::normalize(glm::cross(ref, z));
    return {glm::vec4(x, 0), glm::vec4(glm::cross(z, x), 0), glm::vec4(z, 0), glm::vec4(position, 1)};
}

glm::mat4 decalModel(const ShotHit& hit) {
    glm::vec3 center = hit.point + hit.normal * config::kDecalOffset;
    float size = config::kDecalSize;
    for (int i = 0; i < 3; ++i) {
        if (hit.normal[i] == 0) {
            size = std::min(size, hit.bounds.max[i] - hit.bounds.min[i]);
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (hit.normal[i] == 0) {
            center[i] = glm::clamp(center[i], hit.bounds.min[i] + size/2, hit.bounds.max[i] - size/2);
        }
    }
    return glm::scale(frame(center, hit.normal), {size, size, 1});
}
}

WeaponRenderer::WeaponRenderer(const std::filesystem::path& shaders)
    : m_shader(shaders / "weapon.vert", shaders / "weapon.frag"), m_sparks(shaders), m_cube(makeCube()),
      m_cylinder(makeCylinder()), m_quad(makeQuad()) {
    reset();
}

void WeaponRenderer::reset() {
    m_sparks.clear();
    m_marks.clear();
    m_age = config::kTracerSeconds;
    m_hitTime = 0;
    m_bobPhase = 0;
    m_root = glm::translate(glm::mat4{1}, {config::kFlashRight, -config::kFlashDown, -config::kFlashForward});
}

void WeaponRenderer::advance(float dt, float groundDistance, const Weapon& weapon) {
    m_sparks.advance(dt);
    m_age = std::min(m_age + dt, config::kTracerSeconds);
    m_hitTime = std::max(0.0f, m_hitTime - dt);
    m_bobPhase = std::fmod(m_bobPhase + groundDistance * config::kBobFrequency, glm::two_pi<float>());
    const float dip = weapon.reloading() ? std::sin(glm::pi<float>() * weapon.reloadFraction()) : 0.0f;
    const glm::vec3 muzzle{config::kFlashRight + 0.009f * std::sin(m_bobPhase),
                           -config::kFlashDown + 0.005f * std::sin(2*m_bobPhase) - 0.10f*dip,
                           -config::kFlashForward};
    m_root = glm::rotate(glm::translate(glm::mat4{1}, muzzle), -0.45f*dip, {0, 0, 1});
}

void WeaponRenderer::recordShot(const Camera& camera, glm::vec3 end, const std::optional<ShotHit>& hit) {
    // Cùng một gốc nòng cho mô hình, đèn chớp và tracer; inverse(View) đổi từ hệ mắt sang thế giới.
    m_start = glm::vec3(glm::inverse(camera.getViewMatrix()) * m_root[3]);
    m_end = end;
    m_age = 0.0f;
    if (hit) m_sparks.emit(hit->point, hit->normal);
    if (hit && hit->damagedTarget) m_hitTime = config::kHitMarkerSeconds;
    if (hit && !hit->movingTarget && hit->normal != glm::vec3{0}) {
        if (m_marks.size() == config::kMaxDecals) {
            m_marks.pop_front();
        }
        m_marks.push_back(decalModel(*hit));
    }
}

const glm::vec3* WeaponRenderer::flashPosition() const {
    return m_age < config::kFlashSeconds ? &m_start : nullptr;
}

void WeaponRenderer::part(const Mesh& mesh, const glm::mat4& model, glm::vec3 color) {
    m_shader.setMat4("uModel", model);
    m_shader.setVec3("uColor", color);
    mesh.draw();
}

void WeaponRenderer::draw(const Camera& camera, float aspect, bool showSparks) {
    const glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 projection = camera.getProjection(aspect);
    if (showSparks) m_sparks.draw(m_quad, view, projection);
    m_shader.use();
    m_shader.setMat4("uVP", projection * view);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    m_shader.setInt("uMode", 1);
    m_shader.setFloat("uAlpha", 0.9f);
    for (const auto& mark : m_marks) {
        part(m_quad, mark, {0.045f, 0.035f, 0.025f});
    }
    const glm::vec3 segment = m_end - m_start;
    if (m_age < config::kTracerSeconds && glm::dot(segment, segment) > 0.00001f) {
        const float length = glm::length(segment);
        const auto beam = frame((m_start + m_end) * 0.5f, segment / length);
        m_shader.setInt("uMode", 2);
        m_shader.setFloat("uAlpha", 1 - m_age/config::kTracerSeconds);
        part(m_cube, glm::scale(beam, {0.014f, 0.014f, length}), {1, 0.79f, 0.31f});
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // Cảnh đã vẽ xong. Xóa riêng depth để tường sát camera không cắt xuyên mô hình góc nhìn.
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shader.setMat4("uVP", projection);
    m_shader.setInt("uMode", 0);
    m_shader.setFloat("uAlpha", 1.0f);
    const auto block = [&](glm::vec3 offset, glm::vec3 size, glm::vec3 color) {
        part(m_cube, glm::scale(glm::translate(m_root, offset), size), color);
    };
    block({0, -0.015f, 0.24f}, {0.115f, 0.11f, 0.25f}, {0.24f, 0.33f, 0.37f});
    block({0, -0.102f, 0.26f}, {0.075f, 0.14f, 0.095f}, {0.14f, 0.18f, 0.20f});
    block({0, 0.046f, 0.23f}, {0.065f, 0.016f, 0.18f}, {0.24f, 0.85f, 0.78f});
    const auto barrel = glm::rotate(glm::translate(m_root, {0, 0, 0.07f}), glm::half_pi<float>(), {1, 0, 0});
    part(m_cylinder, glm::scale(barrel, {0.065f, 0.14f, 0.065f}), {0.38f, 0.45f, 0.46f});
    if (flashPosition()) {
        // Quad song song màn hình trong hệ mắt; blending cộng mô phỏng nguồn phát sáng.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        m_shader.setInt("uMode", 3);
        m_shader.setFloat("uAlpha", 1.0f - m_age/config::kFlashSeconds);
        part(m_quad, glm::scale(glm::translate(glm::mat4{1}, glm::vec3(m_root[3])), {0.16f, 0.16f, 1}),
             {1.0f, 0.55f, 0.10f});
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}
