#pragma once

#include "gfx/Material.h"
#include <glad/glad.h>
#include <filesystem>
#include <vector>

class Texture {
public:
    Texture(const std::filesystem::path& path, Surface fallback);
    static Texture makeSky();
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
