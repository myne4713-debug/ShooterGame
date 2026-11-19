#pragma once
#include "core/GameScreen.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class Input {
public:
    explicit Input(GLFWwindow* window);
    ~Input();
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    GameScreen screen() const { return m_screen; }
    void setScreen(GameScreen screen);
    bool takeStart();
    glm::vec2 mouseDelta();
    glm::vec2 movement() const;
    bool running() const;
    bool takeJump();
    bool flashlightOn() const { return m_flashlight; }
    bool takeTrigger();
    bool takeReload();
    bool showFps() const { return m_showFps; }
    bool shadowsOn() const { return m_shadows; }
    bool sparksOn() const { return m_sparks; }

private:
    static void onKey(GLFWwindow* window, int key, int scanCode, int action, int mods);
    static void onFocus(GLFWwindow* window, int focused);
    static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    GLFWwindow* m_window;
    glm::dvec2 m_lastMouse{};
    GameScreen m_screen = GameScreen::Menu;
    bool m_start = false;
    bool m_resetMouse = true;
    bool m_jump = false;
    bool m_flashlight = false;
    bool m_firePressed = false;
    bool m_fireHeld = false;
    bool m_reload = false;
    bool m_showFps = false;
    bool m_shadows = true, m_sparks = true;
};
