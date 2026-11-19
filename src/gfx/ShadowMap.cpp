#include "gfx/ShadowMap.h"
#include <stdexcept>
#include <string>
#include <utility>

ShadowMap::ShadowMap(int resolution) : ShadowMap() {
    m_resolution = resolution;
    glGenTextures(1, &m_depth);
    glBindTexture(GL_TEXTURE_2D, m_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, resolution, resolution, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    // Lọc tuyến tính nội suy kết quả so sánh của các texel lân cận để làm mềm rìa bóng.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    constexpr GLfloat border[]{1, 1, 1, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);
    // FBO chỉ lưu mặt gần đèn nhất trên mỗi tia, nên cả hai bộ đệm màu đều để GL_NONE.
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    const auto error = glGetError();
    if (!m_framebuffer || !m_depth || status != GL_FRAMEBUFFER_COMPLETE || error != GL_NO_ERROR) {
        throw std::runtime_error("ShadowMap framebuffer setup failed, status " + std::to_string(status)
                                 + ", OpenGL error " + std::to_string(error));
    }
}

ShadowMap::~ShadowMap() {
    glDeleteFramebuffers(1, &m_framebuffer);
    glDeleteTextures(1, &m_depth);
}

ShadowMap::ShadowMap(ShadowMap&& other) noexcept : ShadowMap() {
    *this = std::move(other);
}

ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept {
    std::swap(m_framebuffer, other.m_framebuffer);
    std::swap(m_depth, other.m_depth);
    std::swap(m_resolution, other.m_resolution);
    return *this;
}

void ShadowMap::bindForWriting() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_resolution, m_resolution);
}

void ShadowMap::bindDepthTexture() const {
    glBindTexture(GL_TEXTURE_2D, m_depth);
}
