#include "gfx/LevelRenderer.h"
#include "core/Config.h"
#include "gfx/Camera.h"
#include "gfx/Primitives.h"
#include "world/Level.h"
#include "world/Enemy.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
};
constexpr std::array<PointLight, 3> kArenaLights{{
    {{-4, 2.8f, 3}, {1.0f, 0.52f, 0.23f}},
    {{4, 3.4f, -1}, {0.30f, 0.65f, 1.0f}},
    {{-3, 3.2f, -8}, {0.95f, 0.73f, 0.41f}}
}};
constexpr int kFlashIndex = static_cast<int>(kArenaLights.size());
static_assert(kFlashIndex + 1 <= 4, "phong.frag has four point light slots");
constexpr glm::vec3 kSunDirection{-0.52f, -1.0f, -0.38f};

glm::mat4 boxModel(glm::vec3 center, glm::vec3 size) {
    return glm::scale(glm::translate(glm::mat4{1}, center), size);
}
}

LevelRenderer::LevelRenderer(const std::filesystem::path& assets)
    : m_phong(assets / "shaders/phong.vert", assets / "shaders/phong.frag"),
      m_skyShader(assets / "shaders/skybox.vert", assets / "shaders/skybox.frag"),
      m_depthShader(assets / "shaders/shadow.vert", assets / "shaders/shadow.frag"),
      m_shadowMap(config::kShadowResolution),
      m_cube(makeCube()), m_floor(makePlane(1.0f)), m_cylinder(makeCylinder()), m_quad(makeQuad()),
      m_textures{{Texture(assets / "textures/floor.png", Surface::Floor),
                  Texture(assets / "textures/wall.png", Surface::Wall),
                  Texture(assets / "textures/wood.png", Surface::Wood),
                  Texture(assets / "textures/metal.png", Surface::Metal)}},
      m_sky(Texture::makeSky()) {
    // Tia mặt trời song song nên dùng phép chiếu trực giao cố định bao trọn đấu trường.
    // Giữ góc nhìn của đèn độc lập với camera để bóng đứng yên khi người chơi xoay đầu.
    const glm::vec3 center{0, config::kWallHeight/2, 0};
    const auto eye = center - glm::normalize(kSunDirection) * config::kSunDistance;
    const float extent = config::kShadowExtent;
    const auto lightVP = glm::ortho(-extent, extent, -extent, extent, 0.1f, 2 * config::kSunDistance)
                       * glm::lookAt(eye, center, {0, 1, 0});
    m_depthShader.use();
    m_depthShader.setMat4("uLightVP", lightVP);
    m_phong.use();
    m_phong.setMat4("uLightVP", lightVP);
    m_phong.setInt("uShadowMap", 1);
    m_phong.setInt("uAlbedo", 0);
    m_phong.setVec3("uAmbient", {0.32f, 0.34f, 0.38f});
    m_phong.setVec3("uSunDirection", kSunDirection);
    m_phong.setVec3("uSunColor", {0.73f, 0.67f, 0.57f});
    m_phong.setVec3("uAttenuation", {1.0f, 0.14f, 0.07f});
    m_phong.setVec3("uSpotColor", {1.9f, 1.82f, 1.53f});
    m_phong.setFloat("uInnerCos", std::cos(glm::radians(config::kSpotInnerDegrees)));
    m_phong.setFloat("uOuterCos", std::cos(glm::radians(config::kSpotOuterDegrees)));
    m_phong.setFloat("uFogStart", config::kFogStart);
    m_phong.setFloat("uFogEnd", config::kFogEnd);
    for (std::size_t i = 0; i < kArenaLights.size(); ++i) {
        const std::string light = "uPoints[" + std::to_string(i) + "]";
        m_phong.setVec3(light + ".position", kArenaLights[i].position);
        m_phong.setVec3(light + ".color", kArenaLights[i].color);
    }
    m_phong.setVec3("uPoints[3].color", {3.0f, 1.6f, 0.52f});
    m_skyShader.use();
    m_skyShader.setInt("uSky", 0);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void LevelRenderer::drawSurface(const Mesh& mesh, const Material& material, glm::vec3 center,
                                glm::vec3 size) {
    m_textures[static_cast<std::size_t>(material.surface)].bind();
    m_phong.setVec3("uMaterial.color", material.color);
    m_phong.setFloat("uMaterial.shininess", material.shininess);
    // Cùng mật độ texel trên sàn và tường dài. Vertex shader chọn hai trục nằm trên mặt.
    m_phong.setVec3("uTiles", size / config::kFloorTileSize);
    m_phong.setMat4("uModel", boxModel(center, size));
    mesh.draw();
}

void LevelRenderer::drawShadowMap(const Level& level, const std::vector<Enemy>& enemies) {
    GLint viewport[4], framebuffer;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    m_shadowMap.bindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    // Khi lọc PCF, texel bên cạnh trên cùng mặt nghiêng có thể gần đèn hơn.
    // Dời độ sâu theo độ dốc trong rasterizer để tránh hoa văn tự che trên mặt đang sáng.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(config::kShadowSlopeBias, config::kShadowDepthUnits);
    m_depthShader.use();
    for (const auto& box : level.boxes()) {
        m_depthShader.setMat4("uModel", boxModel(box.center, box.size));
        m_cube.draw();
    }
    drawEnemies(enemies, glm::mat4{1}, true);
    glDisable(GL_POLYGON_OFFSET_FILL);
    // Shadow map vuông, còn framebuffer cửa sổ có thể vừa đổi cỡ hoặc dùng HiDPI.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(framebuffer));
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void LevelRenderer::drawEnemies(const std::vector<Enemy>& enemies, const glm::mat4& view, bool depthOnly) {
    Shader& shader = depthOnly ? m_depthShader : m_phong;
    if (!depthOnly) {
        m_textures[static_cast<std::size_t>(Surface::Metal)].bind();
        m_phong.setFloat("uMaterial.shininess", config::kMetalShininess);
        m_phong.setVec3("uTiles", {1, 1, 1});
    }
    const auto part = [&](const Mesh& mesh, const glm::mat4& parent, glm::vec3 offset,
                          glm::vec3 size, glm::vec3 color) {
        shader.setMat4("uModel", glm::scale(glm::translate(parent, offset), size));
        if (!depthOnly) shader.setVec3("uMaterial.color", color);
        mesh.draw();
    };
    // Nghịch đảo phần quay của View đưa các trục ngang/dọc màn hình về hệ thế giới.
    // Billboard giữ đúng hướng khi camera xoay và vẫn bị tường che nhờ depth test.
    const glm::mat4 billboard{glm::transpose(glm::mat3(view))};
    const glm::vec3 dark{0.18f, 0.24f, 0.30f};
    for (const Enemy& enemy : enemies) {
        const float fade = enemy.deathScale();
        if (fade <= 0) continue;
        const bool runner = enemy.type() == EnemyType::Runner;
        const glm::vec3 armor = runner ? glm::vec3{1.0f, 0.34f, 0.13f} : glm::vec3{0.23f, 0.59f, 0.98f};
        const float width = config::kEnemyRadius * 2;
        auto root = glm::rotate(glm::translate(glm::mat4{1}, enemy.feet()), enemy.yaw(), {0,1,0});
        root = glm::scale(root, fade * glm::vec3{width, enemy.height(), width});
        part(runner ? m_cylinder : m_cube, root, {0,.58f,0}, {runner ? .58f : .74f,.40f,.48f}, armor);
        part(m_cube, root, {0,.89f,0}, {.52f,.22f,.45f}, dark);
        part(m_cube, root, {0,.90f,.23f}, {.40f,.06f,.04f}, {1,.81f,.28f});
        const float swing = enemy.state() == EnemyState::Chase ? .50f * std::sin(enemy.phase()) : 0;
        for (float side : {-1.f, 1.f}) {
            // Quay tại vai/hông trước khi dịch tâm chi xuống. Hai bên lệch pha để bước chân đối nhau.
            auto shoulder = glm::translate(root, {side*.41f,.72f,0});
            const float reach = enemy.state() == EnemyState::Attack ? -1.0f : side*swing;
            shoulder = glm::rotate(shoulder, reach, {1,0,0});
            part(m_cylinder, shoulder, {0,-.13f,0}, {.16f,.28f,.18f}, armor);
            auto hip = glm::translate(root, {side*.18f,.40f,0});
            hip = glm::rotate(hip, -side*swing, {1,0,0});
            part(m_cube, hip, {0,-.20f,0}, {.24f,.40f,.30f}, dark);
        }
        if (depthOnly || enemy.state() == EnemyState::Dead) continue;
        const auto anchor = enemy.feet() + glm::vec3{0, enemy.height()+.20f, 0};
        const auto bar = glm::translate(glm::mat4{1}, anchor) * billboard;
        const float fraction = static_cast<float>(enemy.health()) / enemy.maxHealth();
        part(m_quad, bar, {0,0,0}, {.92f,.13f,1}, {.12f,.12f,.14f});
        // Giữ cố định mép trái khi thanh ngắn lại. Dịch về phía mắt để tránh tranh chấp độ sâu.
        part(m_quad, bar, {-.43f*(1-fraction),0,.004f}, {.86f*fraction,.08f,1}, {.32f,1,.44f});
    }
}

void LevelRenderer::draw(const Level& level, const Camera& camera, float aspect,
                         bool flashlight, float brightness, const glm::vec3* flashPosition,
                         const std::vector<Enemy>& enemies, bool shadows) {
    const glm::vec3 horizon{config::kClearRed, config::kClearGreen, config::kClearBlue};
    const glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 projection = camera.getProjection(aspect);
    if (shadows) drawShadowMap(level, enemies);
    glActiveTexture(GL_TEXTURE1);
    m_shadowMap.bindDepthTexture();
    m_phong.use();
    m_phong.setInt("uShadows", shadows);
    m_phong.setMat4("uView", view);
    m_phong.setMat4("uProjection", projection);
    m_phong.setVec3("uEye", camera.position());
    m_phong.setVec3("uFront", camera.front());
    m_phong.setInt("uSpotOn", flashlight);
    m_phong.setVec3("uFogColor", horizon * brightness);
    m_phong.setInt("uPointCount", kFlashIndex + (flashPosition ? 1 : 0));
    if (flashPosition) {
        m_phong.setVec3("uPoints[3].position", *flashPosition);
    }
    glActiveTexture(GL_TEXTURE0);
    const Material floor{Surface::Floor, {0.73f, 0.69f, 0.60f}, config::kFloorShininess};
    drawSurface(m_floor, floor, {0, 0, 0}, {config::kArenaSize, 1, config::kArenaSize});
    for (const auto& box : level.boxes()) {
        drawSurface(m_cube, box.material, box.center, box.size);
    }
    drawEnemies(enemies, view, false);

    // Nhìn từ trong cube nên vẽ mặt sau. LEQUAL chỉ tô trời nơi depth vẫn bằng 1.
    m_skyShader.use();
    m_skyShader.setMat4("uView", view);
    m_skyShader.setMat4("uProjection", projection);
    m_skyShader.setFloat("uBrightness", brightness);
    m_sky.bind();
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    m_cube.draw();
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
}
