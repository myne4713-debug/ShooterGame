#include "gfx/Texture.h"
#include "core/Config.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
// Token của EXT/ARB_texture_filter_anisotropic. GLAD của repo chỉ sinh các token Core 3.3.
constexpr GLenum kTextureAnisotropy = 0x84FE, kMaxTextureAnisotropy = 0x84FF;

std::vector<unsigned char> surfacePixels(Surface surface) {
    const int size = config::kTextureSize;
    std::vector<unsigned char> pixels(size * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const int row = y / (size / 4);
            int shade = 0;
            switch (surface) {
            case Surface::Floor:
                shade = (x < 2 || y < 2) ? 132 : 225;
                break;
            case Surface::Wall: {
                const int brickX = (x + (row % 2) * size / 4) % (size / 2);
                shade = (brickX < 2 || y % (size / 4) < 2) ? 155 : 231;
                break;
            }
            case Surface::Wood:
                shade = y % (size / 4) < 2 ? 138 : 208 + static_cast<int>(
                    17 * std::sin(0.5f * y + std::sin(6.2831853f * x / size)));
                break;
            case Surface::Metal:
                shade = x < 2 || y < 2 ? 145 : 221;
                break;
            }
            // Hạt lặp được vì GL_REPEAT lấy lại chính mảng texel này ở ô kế tiếp.
            const unsigned hash = (static_cast<unsigned>(x) * 73856093u)
                                ^ (static_cast<unsigned>(y) * 19349663u);
            const auto value = static_cast<unsigned char>(shade + hash % 11u);
            const auto i = (y * size + x) * 4;
            pixels[i] = pixels[i + 1] = pixels[i + 2] = value;
            pixels[i + 3] = 255;
        }
    }
    return pixels;
}

void checkUpload(const std::string& label) {
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        throw std::runtime_error("Texture upload failed (" + label + "), GL error " + std::to_string(error));
    }
}
}

Texture::Texture(GLenum target) : m_target(target) {
    glGenTextures(1, &m_id);
    if (!m_id) {
        throw std::runtime_error("Could not allocate OpenGL texture.");
    }
    bind();
}

Texture::Texture(const std::filesystem::path& path, Surface fallback) : Texture(GL_TEXTURE_2D) {
    stbi_set_flip_vertically_on_load(true);
    int width = 0, height = 0;
    const auto name = path.string();
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> image(
        stbi_load(name.c_str(), &width, &height, nullptr, STBI_rgb_alpha), stbi_image_free);
    std::vector<unsigned char> generated;
    if (!image) {
        std::printf("Procedural texture for %s (%s)\n", name.c_str(), stbi_failure_reason());
        generated = surfacePixels(fallback);
        width = height = config::kTextureSize;
    }
    // RGBA luôn có stride chia hết cho 4, kể cả ảnh RGB nguồn có chiều rộng lẻ.
    glTexImage2D(m_target, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image ? image.get() : generated.data());
    glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Chọn giữa các mức mipmap để bề mặt nhìn xa ít nhấp nháy. Lọc dị hướng giữ nét khi nhìn xiên.
    glGenerateMipmap(m_target);
    if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic") ||
        glfwExtensionSupported("GL_ARB_texture_filter_anisotropic")) {
        GLfloat limit = 1.0f;
        glGetFloatv(kMaxTextureAnisotropy, &limit);
        glTexParameterf(m_target, kTextureAnisotropy, std::min(limit, config::kAnisotropyLimit));
    }
    checkUpload(name);
}

Texture Texture::makeMask(int width, int height, const std::vector<unsigned char>& pixels) {
    if (width <= 0 || height <= 0 || pixels.size() != static_cast<std::size_t>(width) * height) {
        throw std::invalid_argument("Invalid bitmap mask size");
    }
    Texture texture(GL_TEXTURE_2D);
    GLint alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkUpload("bitmap mask");
    return texture;
}

Texture Texture::makeSky() {
    Texture sky(GL_TEXTURE_CUBE_MAP);
    const int size = config::kSkyFaceSize;
    const glm::vec3 horizon{config::kClearRed, config::kClearGreen, config::kClearBlue};
    const glm::vec3 zenith{0.16f, 0.39f, 0.67f};
    std::vector<unsigned char> pixels(size * size * 4);
    for (int face = 0; face < 6; ++face) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                const float s = 2.0f * (x + 0.5f) / size - 1.0f;
                const float t = 2.0f * (y + 0.5f) / size - 1.0f;
                // Trong quy ước cubemap GL, bốn mặt bên có hướng Y = -t.
                // Tô theo độ cao của hướng đã chuẩn hóa để màu nối liền tại cạnh.
                const float axisY = face == 2 ? 1.0f : (face == 3 ? -1.0f : -t);
                const float elevation = axisY / std::sqrt(1.0f + s*s + t*t);
                const auto color = glm::mix(horizon, zenith, glm::smoothstep(0.0f, 1.0f, elevation));
                const int i = (y * size + x) * 4;
                for (int channel = 0; channel < 3; ++channel) {
                    pixels[i + channel] = static_cast<unsigned char>(255.0f * color[channel] + 0.5f);
                }
                pixels[i + 3] = 255;
            }
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8, size, size,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }
    glTexParameteri(sky.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(sky.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (const auto axis : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R}) {
        glTexParameteri(sky.m_target, axis, GL_CLAMP_TO_EDGE);
    }
    checkUpload("sky cubemap");
    return sky;
}

Texture::~Texture() {
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
}

Texture::Texture(Texture&& other) noexcept
    : m_target(other.m_target), m_id(std::exchange(other.m_id, 0)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    std::swap(m_target, other.m_target);
    std::swap(m_id, other.m_id);
    return *this;
}

void Texture::bind() const {
    glBindTexture(m_target, m_id);
}
