#include "ui/HUD.h"
#include "core/Config.h"
#include "gfx/Primitives.h"
#include "world/Weapon.h"
#include "world/WaveManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>

namespace {
Texture makeFont() {
    struct Glyph { char letter; unsigned char rows[7]; };
    constexpr Glyph glyphs[] = {
        {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}},
        {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}},
        {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
        {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}},
        {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
        {'/',{1,1,2,4,8,16,16}}, {'A',{14,17,17,31,17,17,17}},
        {'D',{30,17,17,17,17,17,30}}, {'E',{31,16,16,30,16,16,31}},
        {'F',{31,16,16,30,16,16,16}}, {'H',{17,17,17,31,17,17,17}},
        {'L',{16,16,16,16,16,16,31}}, {'M',{17,27,21,21,17,17,17}},
        {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
        {'R',{30,17,17,30,20,18,17}}, {'S',{15,16,16,14,1,1,30}},
        {'U',{17,17,17,17,17,17,14}}, {'C',{14,17,16,16,16,17,14}},
        {'G',{14,17,16,23,17,17,15}}, {'I',{14,4,4,4,4,4,14}},
        {'N',{17,25,21,19,17,17,17}}, {'Q',{14,17,17,17,21,18,13}},
        {'T',{31,4,4,4,4,4,4}}, {'V',{17,17,17,17,17,10,4}},
        {'W',{17,17,17,21,21,21,10}}, {'Y',{17,17,10,4,4,4,4}}
    };
    constexpr int cell = 8, width = 128 * cell;
    std::vector<unsigned char> pixels(width * cell, 0);
    for (const auto& glyph : glyphs) {
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (glyph.rows[row] & (1 << (4-col))) {
                    pixels[(7-row)*width + glyph.letter*cell + col] = 255;
                }
            }
        }
    }
    return Texture::makeMask(width, cell, pixels);
}
}

HUD::HUD(const std::filesystem::path& shaders)
    : m_shader(shaders / "hud.vert", shaders / "hud.frag"), m_quad(makeQuad()), m_font(makeFont()) {}

void HUD::drawQuads(glm::vec2 origin, glm::vec2 size, glm::vec3 color, float alpha,
                    const GLint* glyphs, GLsizei count) {
    m_shader.setVec3("uOrigin", {origin, 0});
    m_shader.setVec3("uSize", {size, 1});
    m_shader.setVec3("uColor", color);
    m_shader.setFloat("uAlpha", alpha);
    m_shader.setIntArray("uGlyphs[0]", glyphs, count);
    m_quad.drawInstanced(count);
}

void HUD::rectangle(float x, float y, float w, float h, glm::vec3 color, float alpha) {
    constexpr GLint solid = -1;
    drawQuads({x, y}, {w, h}, color, alpha, &solid, 1);
}

void HUD::text(float x, float y, float pixel, const std::string& value, glm::vec3 color) {
    std::array<GLint, config::kHudGlyphsPerBatch> glyphs;
    // Các chữ liền nhau dùng chung atlas, cỡ và màu. Giữ nguyên thứ tự từng chuỗi
    // để lớp phủ alpha của menu vẫn nằm trên các phần tử được vẽ trước đó.
    for (std::size_t first = 0; first < value.size(); first += glyphs.size()) {
        const auto count = std::min(glyphs.size(), value.size() - first);
        std::copy_n(value.begin() + first, count, glyphs.begin());
        drawQuads({x + first * (6*pixel), y}, {8*pixel, 8*pixel}, color, 1,
                  glyphs.data(), static_cast<GLsizei>(count));
    }
}

void HUD::overlay(float w, float h, GameScreen screen, int score) {
    const glm::vec3 white{.96f}, accent{.45f,.95f,.85f};
    const auto line = [&](float y, float pixel, const std::string& value, glm::vec3 color) {
        text((w - value.size()*6*pixel)/2, h/2+y, pixel, value, color);
    };
    rectangle(0, 0, w, h, {.02f,.045f,.06f}, .78f);
    if (screen == GameScreen::Menu) {
        line(24, 3, "ARENA", white);
        line(0, 1.5f, "SURVIVE THE WAVES", accent);
        line(-26, 1.5f, "ENTER  PLAY", white);
    } else if (screen == GameScreen::Paused) {
        line(18, 3, "PAUSED", white);
        line(-16, 1.5f, "ESC  RESUME", accent);
    } else {
        line(24, 3, "GAME OVER", white);
        line(0, 1.75f, "SCORE " + std::to_string(score), accent);
        line(-26, 1.5f, "ENTER  RESTART", white);
    }
    line(-47, 1.25f, "Q  QUIT", white);
}

void HUD::draw(int width, int height, const Weapon& weapon, const WaveManager& waves,
               bool hit, GameScreen screen, int fps) {
    // Ortho giữ tâm đúng giữa framebuffer; cùng một tỉ lệ trên hai trục để chữ luôn vuông pixel.
    const float scale = std::min(width / 640.0f, height / 360.0f);
    const float w = width / scale, h = height / scale;
    m_shader.use();
    m_shader.setMat4("uProjection", glm::ortho(0.0f, w, 0.0f, h));
    glActiveTexture(GL_TEXTURE0);
    m_font.bind();
    m_shader.setInt("uFont", 0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (screen == GameScreen::Playing || screen == GameScreen::Paused) {
        drawStatus(w, h, weapon, waves, hit);
    }
    if (screen != GameScreen::Playing) overlay(w, h, screen, waves.score());
    if (fps >= 0) {
        rectangle(12, h-27, 76, 15, {.035f,.075f,.09f}, .8f);
        text(16, h-24, 1.25f, "FPS " + std::to_string(fps), glm::vec3{.96f});
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::drawStatus(float w, float h, const Weapon& weapon, const WaveManager& waves, bool hit) {
    const glm::vec3 ink{0.035f, 0.075f, 0.09f}, accent{0.45f, 0.95f, 0.85f}, white{0.96f};
    rectangle(12, 12, 72, 30, ink, 0.82f);
    text(18, 28, 1.25f, "HP", accent);
    text(18, 15, 1.5f, std::to_string(waves.health()), white);
    rectangle(w-103, 12, 91, 30, ink, 0.82f);
    text(w-97, 28, 1.25f, weapon.reloading() ? "RELOAD" : "AMMO", accent);
    text(w-97, 15, 1.5f, std::to_string(weapon.magazine()) + " / " + std::to_string(weapon.reserve()), white);
    if (weapon.reloading()) {
        rectangle(w-103, 12, 91 * weapon.reloadFraction(), 1.5f, accent);
    }
    const glm::vec3 centerColor = hit ? glm::vec3{1, 0.54f, 0.14f} : white;
    const float x = w/2, y = h/2;
    const auto bar = [&](float bx, float by, float bw, float bh) {
        rectangle(bx-0.5f, by-0.5f, bw+1, bh+1, ink, 0.8f);
        rectangle(bx, by, bw, bh, centerColor);
    };
    bar(x-6, y-0.5f, 4, 1);
    bar(x+2, y-0.5f, 4, 1);
    bar(x-0.5f, y-6, 1, 4);
    bar(x-0.5f, y+2, 1, 4);
    const std::string stats = "WAVE " + std::to_string(waves.wave()) + "   SCORE " + std::to_string(waves.score());
    const float statsWidth = stats.size()*7.5f + 12;
    rectangle(w-12-statsWidth, h-27, statsWidth, 15, ink, .82f);
    text(w-6-statsWidth, h-24, 1.25f, stats, accent);
    if (waves.damageFraction() > 0) {
        // Trộn viền đỏ sau cảnh 3D. Alpha giảm theo thời gian mô phỏng nên pause giữ nguyên hiệu ứng.
        const float alpha = .65f * waves.damageFraction();
        const glm::vec3 red{.92f,.05f,.04f};
        constexpr float border = 9;
        rectangle(0, 0, w, border, red, alpha);
        rectangle(0, h-border, w, border, red, alpha);
        rectangle(0, border, border, h-2*border, red, alpha);
        rectangle(w-border, border, border, h-2*border, red, alpha);
    }
}
