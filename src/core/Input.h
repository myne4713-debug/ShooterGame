#pragma once
#include "core/GameScreen.h"
#include "world/MapCatalog.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <cstddef>
#include <filesystem>
#include <optional>

class Input {
public:
    explicit Input(GLFWwindow* window);
    ~Input();
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    GameScreen screen() const { return m_screen; }
    void setScreen(GameScreen screen);
    MapId selectedMap() const { return m_selectedMap; }
    void setCustomMapAvailable(bool available);
    void selectCustomMap();
    bool takeMapChanged();
    std::optional<std::filesystem::path> takeDroppedPath();
    bool takeOpenImageRequest();
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
    void selectMap(std::size_t index);
    void selectMapSlot(std::size_t slot);
    static void onKey(GLFWwindow* window, int key, int scanCode, int action, int mods);
    static void onFocus(GLFWwindow* window, int focused);
    static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void onDrop(GLFWwindow* window, int count, const char** paths);
    GLFWwindow* m_window;
    glm::dvec2 m_lastMouse{};
    GameScreen m_screen = GameScreen::Menu;
    MapId m_selectedMap = MapId::Arena;
    bool m_hasCustomMap = false;
    bool m_mapChanged = false;
    bool m_openImageRequest = false;
    std::optional<std::filesystem::path> m_droppedPath;
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
