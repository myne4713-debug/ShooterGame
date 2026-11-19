#pragma once

#include "core/Config.h"
#include "gfx/Mesh.h"
#include "gfx/ImpactSparks.h"
#include "gfx/Shader.h"
#include "world/Level.h"
#include <deque>

class Camera;
class Weapon;

class WeaponRenderer {
public:
    explicit WeaponRenderer(const std::filesystem::path& shaders);
    void reset();
    void advance(float dt, float groundDistance, const Weapon& weapon);
    void recordShot(const Camera& camera, glm::vec3 end, const std::optional<ShotHit>& hit);
    const glm::vec3* flashPosition() const;
    bool hitMarker() const { return m_hitTime > 0.0f; }
    void draw(const Camera& camera, float aspect, bool showSparks);

private:
    void part(const Mesh& mesh, const glm::mat4& model, glm::vec3 color);
    Shader m_shader;
    ImpactSparks m_sparks;
    Mesh m_cube, m_cylinder, m_quad;
    std::deque<glm::mat4> m_marks;
    glm::vec3 m_start{}, m_end{};
    glm::mat4 m_root{1};
    float m_age = config::kTracerSeconds;
    float m_hitTime = 0.0f;
    float m_bobPhase = 0.0f;
};
