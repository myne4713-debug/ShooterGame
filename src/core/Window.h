#pragma once

#include <memory>
#include <GLFW/glfw3.h>

class Window {
public:
    Window();
    GLFWwindow* handle() const { return m_handle.get(); }

private:
    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> m_handle{nullptr, glfwDestroyWindow};
};
