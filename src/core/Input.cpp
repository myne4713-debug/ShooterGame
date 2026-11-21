#include "core/Input.h"
#include "core/Config.h"
#include "ui/HUD.h"

#include <utility>

Input::Input(GLFWwindow* window) : m_window(window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetWindowFocusCallback(window, onFocus);
    glfwSetDropCallback(window, onDrop);
    setScreen(GameScreen::Menu);
}

Input::~Input() {
    glfwSetKeyCallback(m_window, nullptr);
    glfwSetMouseButtonCallback(m_window, nullptr);
    glfwSetWindowFocusCallback(m_window, nullptr);
    glfwSetDropCallback(m_window, nullptr);
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
    if (screen == GameScreen::Menu) title = "DHMT - Chọn map (1-5 / Mũi tên) | O / Kéo thả ảnh | Enter: Chơi";
    if (screen == GameScreen::Paused) title = "Tạm dừng | Esc: Tiếp tục | M: Đổi map | Q: Thoát";
    if (screen == GameScreen::GameOver) title = "DHMT - Game Over | Enter: Chơi lại | M: Đổi map | Q: Thoát";
    glfwSetWindowTitle(m_window, title);
}

void Input::setCustomMapAvailable(bool available) {
    m_hasCustomMap = available;
}

void Input::selectCustomMap() {
    if (!m_hasCustomMap) return;
    if (m_selectedMap != MapId::Custom) {
        m_selectedMap = MapId::Custom;
        m_mapChanged = true;
    }
}

void Input::selectMap(std::size_t index) {
    const MapId next = mapIdFromIndex(index);
    if (next != m_selectedMap) {
        m_selectedMap = next;
        m_mapChanged = true;
    }
}

void Input::selectMapSlot(std::size_t slot) {
    if (slot < kMapCount) {
        selectMap(slot);
    } else if (m_hasCustomMap) {
        selectCustomMap();
    } else {
        m_openImageRequest = true;
    }
}

bool Input::takeMapChanged() {
    return std::exchange(m_mapChanged, false);
}

std::optional<std::filesystem::path> Input::takeDroppedPath() {
    return std::exchange(m_droppedPath, std::nullopt);
}

bool Input::takeOpenImageRequest() {
    return std::exchange(m_openImageRequest, false);
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
    if (screen == GameScreen::Menu) {
        const std::size_t totalMaps = kMapCount + (input.m_hasCustomMap ? 1 : 0);
        const std::size_t current = input.m_selectedMap == MapId::Custom
            ? kMapCount
            : mapIndex(input.m_selectedMap);
        if (key == GLFW_KEY_UP || key == GLFW_KEY_LEFT || key == GLFW_KEY_W || key == GLFW_KEY_A) {
            input.selectMapSlot((current + totalMaps - 1) % totalMaps);
            return;
        }
        if (key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT || key == GLFW_KEY_S || key == GLFW_KEY_D) {
            input.selectMapSlot((current + 1) % totalMaps);
            return;
        }
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5) {
            input.selectMapSlot(static_cast<std::size_t>(key - GLFW_KEY_1));
            return;
        }
        if (key >= GLFW_KEY_KP_1 && key <= GLFW_KEY_KP_5) {
            input.selectMapSlot(static_cast<std::size_t>(key - GLFW_KEY_KP_1));
            return;
        }
        if (key == GLFW_KEY_O) {
            input.m_openImageRequest = true;
            return;
        }
    }
    if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
        && (screen == GameScreen::Menu || screen == GameScreen::GameOver)) {
        input.m_start = true;
    } else if (key == GLFW_KEY_M && (screen == GameScreen::Paused || screen == GameScreen::GameOver)) {
        input.setScreen(GameScreen::Menu);
        input.m_mapChanged = true;
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
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (input.screen() == GameScreen::Playing) {
        input.m_fireHeld = action == GLFW_PRESS;
        input.m_firePressed |= input.m_fireHeld;
        return;
    }
    if (input.screen() == GameScreen::Menu && action == GLFW_PRESS) {
        double cx = 0.0, cy = 0.0;
        int winW = 0, winH = 0, fbW = 0, fbH = 0;
        glfwGetCursorPos(window, &cx, &cy);
        glfwGetWindowSize(window, &winW, &winH);
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (winW > 0 && winH > 0 && fbW > 0 && fbH > 0) {
            const float px = static_cast<float>(cx * fbW / winW);
            const float py = static_cast<float>(fbH - cy * fbH / winH);
            const auto hit = HUD::hitTestMenu(fbW, fbH, px, py);
            if (hit.mapIndex) {
                input.selectMapSlot(*hit.mapIndex);
            } else if (hit.loadImage) {
                input.m_openImageRequest = true;
            } else if (hit.play) {
                input.m_start = true;
            }
        }
    }
}

void Input::onDrop(GLFWwindow* window, int count, const char** paths) {
    if (count <= 0 || !paths || !paths[0]) return;
    auto& input = *static_cast<Input*>(glfwGetWindowUserPointer(window));
    input.m_droppedPath = std::filesystem::u8path(paths[0]);
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
