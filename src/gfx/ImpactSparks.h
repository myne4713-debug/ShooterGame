#pragma once
#include "gfx/Shader.h"
#include <deque>

class Mesh;

class ImpactSparks {
public:
    explicit ImpactSparks(const std::filesystem::path& shaders);
    void emit(glm::vec3 position, glm::vec3 normal);
    void advance(float dt);
    void clear() { m_bursts.clear(); }
    void draw(const Mesh& quad, const glm::mat4& view, const glm::mat4& projection);

private:
    struct Burst {
        glm::vec3 position, normal;
        float age;
    };
    Shader m_shader;
    std::deque<Burst> m_bursts;
};
