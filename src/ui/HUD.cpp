#include "ui/HUD.h"
#include "core/Config.h"
#include "gfx/Primitives.h"
#include "world/Weapon.h"
#include "world/WaveManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>

namespace {
struct VirtualViewport {
    float scale, w, h;
};

struct MenuLayout {
    float panelX, panelY, panelW, panelH;
    float cardX, cardW, cardH, cardGap, firstCardTop;
    float loadX, playX, btnY, btnW, btnH;

    float cardY(std::size_t index) const {
        return firstCardTop - cardH - static_cast<float>(index) * (cardH + cardGap);
    }
};

VirtualViewport virtualViewport(int width, int height) {
    // Ortho giữ tâm đúng giữa framebuffer; cùng một tỉ lệ trên hai trục để chữ luôn vuông pixel.
    const float scale = std::min(width / 640.0f, height / 360.0f);
    return {scale, width / scale, height / scale};
}

MenuLayout makeMenuLayout(float w, float h) {
    const float panelW = 340.0f, panelH = 304.0f;
    const float panelX = (w - panelW) * 0.5f;
    const float panelY = (h - panelH) * 0.5f;
    const float panelTop = panelY + panelH;
    const float cardW = 304.0f, cardH = 28.0f, cardGap = 5.0f;
    const float cardX = (w - cardW) * 0.5f;
    const float btnW = 144.0f, btnH = 24.0f, btnGap = 16.0f;
    return {
        panelX, panelY, panelW, panelH,
        cardX, cardW, cardH, cardGap, panelTop - 52.0f,
        cardX, cardX + btnW + btnGap, panelTop - 262.0f, btnW, btnH
    };
}

std::string selectedMapName(MapId selectedMap, const HUD::CustomMapView* customMap) {
    if (selectedMap == MapId::Custom) {
        return (customMap && customMap->loaded && !customMap->name.empty())
            ? customMap->name
            : std::string{"IMAGE MAP"};
    }
    return mapDefinition(selectedMap).name;
}

Texture makeFont() {
    struct Glyph { char letter; unsigned char rows[7]; };
    constexpr Glyph glyphs[] = {
        {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}},
        {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}},
        {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
        {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}},
        {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
        {'/',{1,1,2,4,8,16,16}}, {'-',{0,0,0,31,0,0,0}},
        {':',{0,4,4,0,4,4,0}}, {'.',{0,0,0,0,0,12,12}},
        {'<',{2,4,8,16,8,4,2}}, {'>',{8,4,2,1,2,4,8}},
        {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
        {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}},
        {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}},
        {'G',{14,17,16,23,17,17,15}}, {'H',{17,17,17,31,17,17,17}},
        {'I',{14,4,4,4,4,4,14}}, {'J',{7,2,2,2,2,18,12}},
        {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}},
        {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
        {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
        {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}},
        {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
        {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}},
        {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}},
        {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}}
    };
    constexpr int cell = 8, width = 128 * cell;
    std::vector<unsigned char> pixels(width * cell, 0);
    for (const auto& glyph : glyphs) {
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (glyph.rows[row] & (1 << (4 - col))) {
                    pixels[(7 - row) * width + glyph.letter * cell + col] = 255;
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
        drawQuads({x + first * (6 * pixel), y}, {8 * pixel, 8 * pixel}, color, 1,
                  glyphs.data(), static_cast<GLsizei>(count));
    }
}

HUD::MenuHit HUD::hitTestMenu(int width, int height, float pixelX, float pixelY) {
    MenuHit hit;
    if (width <= 0 || height <= 0) return hit;
    const VirtualViewport vp = virtualViewport(width, height);
    const MenuLayout layout = makeMenuLayout(vp.w, vp.h);
    // Quy đổi tọa độ pixel sang không gian trực giao ảo dùng chung với lúc vẽ menu.
    const float vx = pixelX / vp.scale;
    const float vy = pixelY / vp.scale;
    const auto inside = [vx, vy](float x, float y, float bw, float bh) {
        return vx >= x && vx <= x + bw && vy >= y && vy <= y + bh;
    };
    for (std::size_t i = 0; i <= kMapCount; ++i) {
        if (inside(layout.cardX, layout.cardY(i), layout.cardW, layout.cardH)) {
            hit.mapIndex = i;
            return hit;
        }
    }
    if (inside(layout.loadX, layout.btnY, layout.btnW, layout.btnH)) {
        hit.loadImage = true;
    } else if (inside(layout.playX, layout.btnY, layout.btnW, layout.btnH)) {
        hit.play = true;
    }
    return hit;
}

void HUD::drawMenu(float w, float h, MapId selectedMap, const CustomMapView* customMap) {
    const glm::vec3 white{0.96f}, accent{0.45f, 0.95f, 0.85f}, muted{0.68f, 0.78f, 0.82f};
    const glm::vec3 warn{1.0f, 0.56f, 0.24f};
    const MenuLayout layout = makeMenuLayout(w, h);
    // Giữ nền mờ nhẹ để người chơi thấy trực tiếp cảnh 3D của map đang chọn phía sau.
    rectangle(0, 0, w, h, {0.02f, 0.045f, 0.06f}, 0.45f);
    rectangle(layout.panelX, layout.panelY, layout.panelW, layout.panelH, {0.025f, 0.055f, 0.075f}, 0.86f);
    rectangle(layout.panelX, layout.panelY + layout.panelH - 2.5f, layout.panelW, 2.5f, accent, 0.95f);

    const auto centeredAt = [&](float cx, float y, float pixel, const std::string& value, glm::vec3 color) {
        const float textW = (static_cast<float>(value.size()) * 6.0f - 1.0f) * pixel;
        text(cx - textW * 0.5f, y, pixel, value, color);
    };
    const float panelTop = layout.panelY + layout.panelH;
    centeredAt(w * 0.5f, panelTop - 28.0f, 2.0f, "FPS 3D ARENA", white);
    centeredAt(w * 0.5f, panelTop - 42.0f, 1.05f, "SELECT MAP OR LOAD IMAGE", accent);

    const std::size_t activeIndex = selectedMap == MapId::Custom ? kMapCount : mapIndex(selectedMap);
    const auto drawCard = [&](std::size_t i, const std::string& name, const std::string& subtitle,
                              const std::string& rightTag) {
        const bool active = i == activeIndex;
        const float cy = layout.cardY(i);
        const glm::vec3 cardColor = active ? glm::vec3{0.10f, 0.22f, 0.27f} : glm::vec3{0.055f, 0.105f, 0.135f};
        rectangle(layout.cardX, cy, layout.cardW, layout.cardH, cardColor, active ? 0.95f : 0.88f);
        if (active) {
            rectangle(layout.cardX, cy, 3.0f, layout.cardH, accent, 1.0f);
        }
        const std::string title = (active ? "> " : "  ") + std::to_string(i + 1) + ". " + name;
        text(layout.cardX + 10.0f, cy + 15.0f, 1.20f, title, active ? white : muted);
        const float tagW = (static_cast<float>(rightTag.size()) * 6.0f - 1.0f) * 1.05f;
        text(layout.cardX + layout.cardW - 10.0f - tagW, cy + 15.5f, 1.05f, rightTag, active ? accent : muted);
        text(layout.cardX + 22.0f, cy + 4.5f, 0.90f, subtitle, active ? accent : muted);
    };

    for (std::size_t i = 0; i < kMapCount; ++i) {
        const auto& def = mapDefinition(i);
        const std::string sizeText = std::to_string(static_cast<int>(def.arenaSize.x)) + "X"
                                   + std::to_string(static_cast<int>(def.arenaSize.y));
        drawCard(i, def.name, def.subtitle, sizeText);
    }
    if (customMap && customMap->loaded) {
        const std::string sizeText = std::to_string(static_cast<int>(customMap->arenaSize.x)) + "X"
                                   + std::to_string(static_cast<int>(customMap->arenaSize.y));
        drawCard(kMapCount, customMap->name, customMap->subtitle, sizeText);
    } else {
        drawCard(kMapCount, "IMAGE MAP", "DROP IMAGE OR PRESS O", "IMG");
    }

    if (customMap && !customMap->statusMessage.empty()) {
        centeredAt(w * 0.5f, panelTop - 230.0f, 0.95f, customMap->statusMessage,
                   customMap->statusError ? warn : accent);
    }

    rectangle(layout.loadX, layout.btnY, layout.btnW, layout.btnH, {0.09f, 0.21f, 0.28f}, 0.95f);
    rectangle(layout.loadX, layout.btnY, layout.btnW, 2.0f, muted, 0.85f);
    centeredAt(layout.loadX + layout.btnW * 0.5f, layout.btnY + 7.0f, 1.15f, "O  LOAD IMAGE", white);

    rectangle(layout.playX, layout.btnY, layout.btnW, layout.btnH, {0.13f, 0.36f, 0.34f}, 0.95f);
    rectangle(layout.playX, layout.btnY, layout.btnW, 2.0f, accent, 0.95f);
    centeredAt(layout.playX + layout.btnW * 0.5f, layout.btnY + 6.5f, 1.20f, "ENTER  PLAY", white);

    centeredAt(w * 0.5f, panelTop - 288.0f, 0.95f, "1-5 / ARROWS MAP   O IMAGE   Q QUIT", muted);
}

void HUD::overlay(float w, float h, GameScreen screen, int score,
                  MapId selectedMap, const CustomMapView* customMap) {
    if (screen == GameScreen::Menu) {
        drawMenu(w, h, selectedMap, customMap);
        return;
    }
    const glm::vec3 white{0.96f}, accent{0.45f, 0.95f, 0.85f};
    const auto line = [&](float y, float pixel, const std::string& value, glm::vec3 color) {
        text((w - value.size() * 6 * pixel) / 2, h / 2 + y, pixel, value, color);
    };
    const std::string mapLabel = "MAP: " + selectedMapName(selectedMap, customMap);
    rectangle(0, 0, w, h, {0.02f, 0.045f, 0.06f}, 0.78f);
    if (screen == GameScreen::Paused) {
        line(30, 3, "PAUSED", white);
        line(4, 1.35f, mapLabel, accent);
        line(-22, 1.5f, "ESC  RESUME", white);
        line(-44, 1.35f, "M  CHANGE MAP", accent);
        line(-66, 1.25f, "Q  QUIT", white);
    } else {
        line(38, 3, "GAME OVER", white);
        line(12, 1.35f, mapLabel, white);
        line(-12, 1.75f, "SCORE " + std::to_string(score), accent);
        line(-36, 1.5f, "ENTER  RESTART", white);
        line(-56, 1.35f, "M  CHANGE MAP", accent);
        line(-76, 1.25f, "Q  QUIT", white);
    }
}

void HUD::draw(int width, int height, const Weapon& weapon, const WaveManager& waves,
               bool hit, GameScreen screen, int fps, MapId selectedMap, const CustomMapView* customMap) {
    const VirtualViewport vp = virtualViewport(width, height);
    const float w = vp.w, h = vp.h;
    m_shader.use();
    m_shader.setMat4("uProjection", glm::ortho(0.0f, w, 0.0f, h));
    glActiveTexture(GL_TEXTURE0);
    m_font.bind();
    m_shader.setInt("uFont", 0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (screen == GameScreen::Playing || screen == GameScreen::Paused) {
        drawStatus(w, h, weapon, waves, hit, selectedMapName(selectedMap, customMap));
    }
    if (screen != GameScreen::Playing) overlay(w, h, screen, waves.score(), selectedMap, customMap);
    if (fps >= 0) {
        rectangle(12, h - 27, 76, 15, {0.035f, 0.075f, 0.09f}, 0.8f);
        text(16, h - 24, 1.25f, "FPS " + std::to_string(fps), glm::vec3{0.96f});
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::drawStatus(float w, float h, const Weapon& weapon, const WaveManager& waves,
                     bool hit, const std::string& mapName) {
    const glm::vec3 ink{0.035f, 0.075f, 0.09f}, accent{0.45f, 0.95f, 0.85f}, white{0.96f};
    rectangle(12, 12, 72, 30, ink, 0.82f);
    text(18, 28, 1.25f, "HP", accent);
    text(18, 15, 1.5f, std::to_string(waves.health()), white);
    rectangle(w - 103, 12, 91, 30, ink, 0.82f);
    text(w - 97, 28, 1.25f, weapon.reloading() ? "RELOAD" : "AMMO", accent);
    text(w - 97, 15, 1.5f, std::to_string(weapon.magazine()) + " / " + std::to_string(weapon.reserve()), white);
    if (weapon.reloading()) {
        rectangle(w - 103, 12, 91 * weapon.reloadFraction(), 1.5f, accent);
    }
    const glm::vec3 centerColor = hit ? glm::vec3{1, 0.54f, 0.14f} : white;
    const float x = w / 2, y = h / 2;
    const auto bar = [&](float bx, float by, float bw, float bh) {
        rectangle(bx - 0.5f, by - 0.5f, bw + 1, bh + 1, ink, 0.8f);
        rectangle(bx, by, bw, bh, centerColor);
    };
    bar(x - 6, y - 0.5f, 4, 1);
    bar(x + 2, y - 0.5f, 4, 1);
    bar(x - 0.5f, y - 6, 1, 4);
    bar(x - 0.5f, y + 2, 1, 4);

    const float badgeWidth = mapName.size() * 6.6f + 12.0f;
    rectangle((w - badgeWidth) * 0.5f, h - 27, badgeWidth, 15, ink, 0.82f);
    text((w - badgeWidth) * 0.5f + 6.0f, h - 24, 1.1f, mapName, white);

    const std::string stats = "WAVE " + std::to_string(waves.wave()) + "   SCORE " + std::to_string(waves.score());
    const float statsWidth = stats.size() * 7.5f + 12;
    rectangle(w - 12 - statsWidth, h - 27, statsWidth, 15, ink, 0.82f);
    text(w - 6 - statsWidth, h - 24, 1.25f, stats, accent);
    if (waves.damageFraction() > 0) {
        // Trộn viền đỏ sau cảnh 3D. Alpha giảm theo thời gian mô phỏng nên pause giữ nguyên hiệu ứng.
        const float alpha = 0.65f * waves.damageFraction();
        const glm::vec3 red{0.92f, 0.05f, 0.04f};
        constexpr float border = 9;
        rectangle(0, 0, w, border, red, alpha);
        rectangle(0, h - border, w, border, red, alpha);
        rectangle(0, border, border, h - 2 * border, red, alpha);
        rectangle(w - border, border, border, h - 2 * border, red, alpha);
    }
}
