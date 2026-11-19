#include "core/Window.h"
#include "gfx/Camera.h"
#include "gfx/LevelRenderer.h"
#include "gfx/WeaponRenderer.h"
#include "ui/HUD.h"
#include "world/WaveManager.h"
#include "world/Weapon.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <stdexcept>

namespace {
constexpr int kWarmupFrames = 30, kMeasuredFrames = 180;

void prepareCrowdedWave(WaveManager& waves, Level& level, glm::vec3 eye) {
    while (waves.enemies().size() < config::kMaxWaveCount) {
        for (const auto& enemy : waves.enemies()) {
            const auto origin = enemy.feet() + glm::vec3{0, 1, 0.6f};
            const auto hit = waves.hitScan(level, origin, {0, 0, -1}, enemy.health());
            if (!hit || !hit->movingTarget) throw std::runtime_error("Cannot prepare benchmark wave");
        }
        if (!waves.step(config::kWaveDelay + static_cast<float>(config::kPhysicsStep), eye, level)) {
            throw std::runtime_error("Benchmark wave did not advance");
        }
    }
}

void runBenchmark() {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    Window window;
    glfwSwapInterval(0);
    constexpr int width = config::kWindowWidth, height = config::kWindowHeight;
    int availableWidth, availableHeight;
    glfwGetFramebufferSize(window.handle(), &availableWidth, &availableHeight);
    if (availableWidth < width || availableHeight < height) {
        throw std::runtime_error("Framebuffer is smaller than the benchmark viewport");
    }
    glViewport(0, 0, width, height);
    const float aspect = static_cast<float>(width) / height;
    const auto shaders = std::filesystem::path(ASSET_DIR) / "shaders";
    LevelRenderer renderer(ASSET_DIR);
    WeaponRenderer effects(shaders);
    HUD hud(shaders);
    Camera camera;
    Level level;
    WaveManager waves(level);
    Weapon weapon;
    prepareCrowdedWave(waves, level, camera.position());
    // Giữ nguyên tư thế robot và tuổi hạt trong mọi mẫu để F6/F7 là hai biến duy nhất.
    const auto dir = glm::normalize(glm::vec3{0, 1.2f, 2} - camera.position());
    const auto hit = level.hitScan(camera.position(), dir, weapon.damage());
    if (!hit) throw std::runtime_error("Benchmark impact is missing");
    weapon.step(0, true, false);
    weapon.step(config::kPhysicsStep, false, true);
    effects.recordShot(camera, hit->point, hit);
    effects.advance(0.1f, 0, weapon);
    std::printf("%s | %dx%d | wave %d | %zu robots\n",
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)), width, height,
                waves.wave(), waves.enemies().size());
    for (bool shadows : {false, true}) {
        for (bool sparks : {false, true}) {
            std::array<double, kMeasuredFrames> samples{};
            for (int i = -kWarmupFrames; i < kMeasuredFrames; ++i) {
                const auto start = std::chrono::steady_clock::now();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                renderer.draw(level, camera, aspect, true, 1, effects.flashPosition(), waves.enemies(), shadows);
                effects.draw(camera, aspect, sparks);
                hud.draw(width, height, weapon, waves, true, GameScreen::Playing, 60);
                // Chờ GPU xong để đo cả lượt kết xuất. Game thật đồng bộ khi đổi buffer bằng VSync.
                glFinish();
                if (i >= 0) {
                    samples[i] = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - start).count();
                }
            }
            std::sort(samples.begin(), samples.end());
            const double median = (samples[kMeasuredFrames/2 - 1] + samples[kMeasuredFrames/2]) / 2;
            const double p95 = samples[(95 * kMeasuredFrames + 99) / 100 - 1];
            std::printf("F6=%d F7=%d | median %.3f ms | p95 %.3f ms\n", shadows, sparks, median, p95);
        }
    }
    if (glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL error in render benchmark");
}
}

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "GLFW initialization failed\n");
        return 1;
    }
    int status = 0;
    try {
        runBenchmark();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        status = 1;
    }
    glfwTerminate();
    return status;
}
