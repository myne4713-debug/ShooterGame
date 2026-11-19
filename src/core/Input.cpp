#include "core/Input.h"
#include "core/Config.h"

#include <utility>

Input::Input(GLFWwindow* window) : m_window(window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetWindowFocusCallback(window, onFocus);
    setScreen(GameScreen::Menu);
}

Input::~Input() {
    glfwSetKeyCallback(m_window, nullptr);
    glfwSetMouseButtonCallback(m_window, nullptr);
    glfwSetWindowFocusCallback(m_window, nullptr);
    glfwSetWindowUserPointer(m_window, nullptr);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Input::setScreen(GameScreen screen) {
    m_screen = screen;
    m_start = false;
    m_jump = false;
    m_firePressed = false;
    m_fireHeld = false;
    m_reload = false;
    m_resetMouse = true;
    const bool playing = screen == GameScreen::Playing;
    glfwSetInputMode(m_window, GLFW_CURSOR, playing ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    const char* title = config::kWindowTitle;
    if (screen == GameScreen::Menu) title = "DHMT - Menu";
    if (screen == GameScreen::Paused) title = "Tạm dừng | Esc: Tiếp tục | Q: Thoát";
    if (screen == GameScreen::GameOver) title = "DHMT - Game Over";
    glfwSetWindowTitle(m_window, title);
}

bool Input::takeStart() {
    if (!std::exchange(m_start, false)) return false;
    m_flashlight = false;
    setScreen(GameScreen::Playing);
    return true;
}

void Input::onKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) {
        return;
    }
    auto& input = *static_cast<Input*>(glfwGetWindowUserPointer(window));
    const auto screen = input.screen();
    const bool playing = screen == GameScreen::Playing;
    if (key == GLFW_KEY_ENTER && (screen == GameScreen::Menu || screen == GameScreen::GameOver)) {
        input.m_start = true;
    } else if (key == GLFW_KEY_ESCAPE && (playing || screen == GameScreen::Paused)) {
        input.setScreen(playing ? GameScreen::Paused : GameScreen::Playing);
    } else if (key == GLFW_KEY_Q && !playing) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_SPACE && playing) {
        input.m_jump = true;
    } else if (key == GLFW_KEY_F && playing) {
        input.m_flashlight = !input.m_flashlight;
    } else if (key == GLFW_KEY_R && playing) {
        input.m_reload = true;
    } else if (key == GLFW_KEY_F3) {
        input.m_showFps = !input.m_showFps;
    } else if (key == GLFW_KEY_F6) {
        input.m_shadows = !input.m_shadows;
    } else if (key == GLFW_KEY_F7) {
        input.m_sparks = !input.m_sparks;
    }
}

void Input::onMouseButton(GLFWwindow* window, int button, int action, int) {
    auto& input = *static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && input.screen() == GameScreen::Playing) {
        input.m_fireHeld = action == GLFW_PRESS;
        input.m_firePressed |= input.m_fireHeld;
    }
}

bool Input::takeTrigger() {
    return std::exchange(m_firePressed, false) || m_fireHeld;
}

bool Input::takeReload() {
    return std::exchange(m_reload, false);
}

void Input::onFocus(GLFWwindow* window, int focused) {
    auto& input = *static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (!focused) {
        input.m_start = false;
        if (input.screen() == GameScreen::Playing) input.setScreen(GameScreen::Paused);
    }
}

glm::vec2 Input::mouseDelta() {
    glm::dvec2 current;
    glfwGetCursorPos(m_window, &current.x, &current.y);
    const glm::vec2 delta = glm::vec2(current - m_lastMouse);
    m_lastMouse = current;
    // Lần đọc sau khi bắt lại con trỏ chỉ làm mốc, tránh xoay theo vị trí chuột ngoài game.
    return std::exchange(m_resetMouse, false) ? glm::vec2{0.0f} : delta;
}

glm::vec2 Input::movement() const {
    return {glfwGetKey(m_window, GLFW_KEY_D) - glfwGetKey(m_window, GLFW_KEY_A),
            glfwGetKey(m_window, GLFW_KEY_W) - glfwGetKey(m_window, GLFW_KEY_S)};
}

bool Input::running() const {
    return glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT);
}

bool Input::takeJump() {
    return std::exchange(m_jump, false);
}
