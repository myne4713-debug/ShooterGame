#pragma once
#include "core/GameScreen.h"

#include "gfx/Mesh.h"
#include "gfx/Shader.h"
#include "gfx/Texture.h"
#include <string>

class Weapon;
class WaveManager;

class HUD {
public:
    explicit HUD(const std::filesystem::path& shaders);
    void draw(int width, int height, const Weapon& weapon, const WaveManager& waves,
              bool hit, GameScreen screen, int fps);

private:
    void drawStatus(float w, float h, const Weapon& weapon, const WaveManager& waves, bool hit);
    void overlay(float w, float h, GameScreen screen, int score);
    void drawQuads(glm::vec2 origin, glm::vec2 size, glm::vec3 color, float alpha,
                   const GLint* glyphs, GLsizei count);
    void rectangle(float x, float y, float w, float h, glm::vec3 color, float alpha = 1.0f);
    void text(float x, float y, float pixel, const std::string& value, glm::vec3 color);
    Shader m_shader;
    Mesh m_quad;
    Texture m_font;
};
