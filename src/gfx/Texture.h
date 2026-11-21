#pragma once

#include "core/Config.h"
#include "gfx/Material.h"
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <filesystem>
#include <vector>

class Texture {
public:
    Texture(const std::filesystem::path& path, Surface fallback);
    static Texture makeSky(
        glm::vec3 horizon = {config::kClearRed, config::kClearGreen, config::kClearBlue},
        glm::vec3 zenith = {0.16f, 0.39f, 0.67f});
    static Texture makeMask(int width, int height, const std::vector<unsigned char>& pixels);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    void bind() const;

private:
    explicit Texture(GLenum target);
    GLenum m_target;
    GLuint m_id = 0;
};
