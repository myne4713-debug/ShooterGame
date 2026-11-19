#include "gfx/ImpactSparks.h"
#include "core/Config.h"
#include "gfx/Mesh.h"

ImpactSparks::ImpactSparks(const std::filesystem::path& shaders)
    : m_shader(shaders / "sparks.vert", shaders / "sparks.frag") {
    m_shader.use();
    m_shader.setFloat("uLifetime", config::kSparkLifetime);
}

void ImpactSparks::emit(glm::vec3 position, glm::vec3 normal) {
    if (normal == glm::vec3{0}) return;
    if (m_bursts.size() == config::kMaxSparkBursts) m_bursts.pop_front();
    m_bursts.push_back({position + normal * config::kDecalOffset, normal, 0.0f});
}

void ImpactSparks::advance(float dt) {
    for (auto& burst : m_bursts) burst.age += dt;
    while (!m_bursts.empty() && m_bursts.front().age >= config::kSparkLifetime) {
        m_bursts.pop_front();
    }
}

void ImpactSparks::draw(const Mesh& quad, const glm::mat4& view, const glm::mat4& projection) {
    if (m_bursts.empty()) return;
    m_shader.use();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    // Cộng ánh sáng nên thứ tự hạt không ảnh hưởng kết quả. Depth test của cảnh vẫn che
    // các tia khuất sau cột, còn tắt ghi depth để hạt phía trước không che lẫn nhau.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (const auto& burst : m_bursts) {
        m_shader.setVec3("uOrigin", burst.position);
        m_shader.setVec3("uNormal", burst.normal);
        m_shader.setFloat("uAge", burst.age);
        quad.drawInstanced(config::kSparksPerImpact);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
