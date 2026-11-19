#pragma once
#include <glad/glad.h>

class ShadowMap {
public:
    explicit ShadowMap(int resolution);
    ~ShadowMap();
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&& other) noexcept;
    ShadowMap& operator=(ShadowMap&& other) noexcept;

    void bindForWriting() const;
    void bindDepthTexture() const;

private:
    ShadowMap() = default;
    GLuint m_framebuffer = 0, m_depth = 0;
    int m_resolution = 0;
};
