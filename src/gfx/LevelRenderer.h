#pragma once

#include "gfx/Mesh.h"
#include "gfx/Shader.h"
#include "gfx/ShadowMap.h"
#include "gfx/Texture.h"
#include <array>
#include <vector>

class Camera;
class Level;
class Enemy;

class LevelRenderer {
public:
    explicit LevelRenderer(const std::filesystem::path& assets);
    void draw(const Level& level, const Camera& camera, float aspect, bool flashlight, float brightness,
              const glm::vec3* flashPosition, const std::vector<Enemy>& enemies, bool shadows);

private:
    void drawShadowMap(const Level& level, const std::vector<Enemy>& enemies);
    void drawEnemies(const std::vector<Enemy>& enemies, const glm::mat4& view, bool depthOnly);
    void drawSurface(const Mesh& mesh, const Material& material, glm::vec3 center, glm::vec3 size);
    Shader m_phong;
    Shader m_skyShader;
    Shader m_depthShader;
    ShadowMap m_shadowMap;
    Mesh m_cube;
    Mesh m_floor;
    Mesh m_cylinder, m_quad;
    std::array<Texture, 4> m_textures;
    Texture m_sky;
};
