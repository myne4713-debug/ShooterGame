#include "core/Window.h"
#include "core/Config.h"
#include "core/GlCheck.h"

#include <glad/glad.h>
#include <cstdio>
#include <stdexcept>

namespace {
void resizeViewport(GLFWwindow*, int width, int height) {
    // Viewport ánh xạ tọa độ chuẩn hóa sang pixel thực của framebuffer, kể cả trên màn hình HiDPI.
    glViewport(0, 0, width, height);
    GL_CHECK();
}

}

Window::Window() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config::kGlMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config::kGlMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    m_handle.reset(glfwCreateWindow(config::kWindowWidth, config::kWindowHeight,
                                    config::kWindowTitle, nullptr, nullptr));
    if (!m_handle) {
        throw std::runtime_error("Could not create an OpenGL 3.3 Core window.");
    }
    glfwMakeContextCurrent(handle());
    const auto loadProc = [](const char* name) -> void* {
        return reinterpret_cast<void*>(glfwGetProcAddress(name));
    };
    if (!gladLoadGLLoader(loadProc) || !GLAD_GL_VERSION_3_3) {
        throw std::runtime_error("GLAD could not load the required OpenGL 3.3 functions.");
    }
    std::printf("OpenGL: %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::fflush(stdout);

    glfwSwapInterval(config::kSwapInterval);
    glfwSetFramebufferSizeCallback(handle(), resizeViewport);
    int width, height;
    glfwGetFramebufferSize(handle(), &width, &height);
    resizeViewport(handle(), width, height);
    // Kiểm tra độ sâu để bề mặt gần che bề mặt xa, bất kể thứ tự vẽ vật thể đặc.
    glEnable(GL_DEPTH_TEST);
    // Với khối kín có thứ tự đỉnh nhất quán, bỏ mặt sau để tránh tô phần khuất.
    glEnable(GL_CULL_FACE);
    GL_CHECK();
}
