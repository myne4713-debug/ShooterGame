#pragma once
#include "core/GameScreen.h"
#include "gfx/Mesh.h"
#include "gfx/Shader.h"
#include "gfx/Texture.h"
#include "world/MapCatalog.h"
#include <glm/vec2.hpp>
#include <cstddef>
#include <optional>
#include <string>

class Weapon;
class WaveManager;

class HUD {
public:
    struct CustomMapView {
        bool loaded = false;
        std::string name;
        std::string subtitle;
        glm::vec2 arenaSize{};
        std::string statusMessage;
        bool statusError = false;
    };

    struct MenuHit {
        std::optional<std::size_t> mapIndex;
        bool loadImage = false;
        bool play = false;
    };

    explicit HUD(const std::filesystem::path& shaders);
    void draw(int width, int height, const Weapon& weapon, const WaveManager& waves,
              bool hit, GameScreen screen, int fps, MapId selectedMap = MapId::Arena,
              const CustomMapView* customMap = nullptr);
    static MenuHit hitTestMenu(int width, int height, float pixelX, float pixelY);

private:
    void drawStatus(float w, float h, const Weapon& weapon, const WaveManager& waves,
                    bool hit, const std::string& mapName);
    void overlay(float w, float h, GameScreen screen, int score,
                 MapId selectedMap, const CustomMapView* customMap);
    void drawMenu(float w, float h, MapId selectedMap, const CustomMapView* customMap);
    void drawQuads(glm::vec2 origin, glm::vec2 size, glm::vec3 color, float alpha,
                   const GLint* glyphs, GLsizei count);
    void rectangle(float x, float y, float w, float h, glm::vec3 color, float alpha = 1.0f);
    void text(float x, float y, float pixel, const std::string& value, glm::vec3 color);
    Shader m_shader;
    Mesh m_quad;
    Texture m_font;
};
