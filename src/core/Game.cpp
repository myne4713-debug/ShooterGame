#include "core/Game.h"
#include "core/Config.h"
#include "core/FileDialog.h"
#include "core/GlCheck.h"
#include "core/Input.h"
#include "core/Window.h"
#include "gfx/Camera.h"
#include "gfx/LevelRenderer.h"
#include "gfx/WeaponRenderer.h"
#include "ui/HUD.h"
#include "world/ImageMapLoader.h"
#include "world/Level.h"
#include "world/Player.h"
#include "world/WaveManager.h"
#include "world/Weapon.h"

#include <glad/glad.h>
#include <algorithm>
#include <cstdio>
#include <exception>

void runGame(Window& window, const std::optional<std::filesystem::path>& initialImage) {
    auto* handle = window.handle();
    LevelRenderer renderer(ASSET_DIR);
    const auto shaders = std::filesystem::path(ASSET_DIR) / "shaders";
    WeaponRenderer effects(shaders);
    HUD hud(shaders);
    Input input(handle);
    Level level(input.selectedMap());
    WaveManager waves(level);
    Weapon weapon;
    Player player(level.playerSpawn());
    Camera camera(player.eyePosition(), level.playerYaw(), level.playerPitch());
    std::optional<MapDefinition> customMapDef;
    HUD::CustomMapView customView;
    std::uint64_t customRevision = 0;
    double physicsTime = 0.0;
    double previousTime = glfwGetTime();
    double elapsed = 0.0;
    double fpsTime = 0.0;
    int frameCount = 0, fps = 0;

    const auto loadMap = [&](MapId mapId) {
        level = (mapId == MapId::Custom && customMapDef) ? Level(*customMapDef) : Level(mapId);
        waves = WaveManager(level);
        player = Player(level.playerSpawn());
        camera = Camera(player.eyePosition(), level.playerYaw(), level.playerPitch());
        weapon = Weapon{};
        effects.reset();
        physicsTime = 0.0;
    };

    const auto tryLoadImageMap = [&](const std::filesystem::path& path) {
        try {
            const auto loaded = loadMapFromImageFile(path, ++customRevision);
            customMapDef = loaded.definition;
            customView = {
                true,
                loaded.definition.name,
                loaded.definition.subtitle,
                loaded.definition.arenaSize,
                "LOADED " + loaded.definition.name + " - "
                    + std::to_string(loaded.gridWidth) + "X" + std::to_string(loaded.gridHeight),
                false,
            };
            input.setCustomMapAvailable(true);
            input.selectCustomMap();
            if (input.screen() != GameScreen::Menu) {
                input.setScreen(GameScreen::Menu);
            }
            input.takeMapChanged();
            loadMap(MapId::Custom);
        } catch (const std::exception& error) {
            std::fprintf(stderr, "Image map load error: %s\n", error.what());
            customView.statusMessage = "CANNOT LOAD IMAGE FILE";
            customView.statusError = true;
        }
    };

    if (initialImage) {
        tryLoadImageMap(*initialImage);
    }

    while (!glfwWindowShouldClose(handle)) {
        glfwPollEvents();
        if (glfwWindowShouldClose(handle)) {
            break;
        }
        if (glfwGetWindowAttrib(handle, GLFW_ICONIFIED)) {
            glfwWaitEvents();
            previousTime = glfwGetTime();
            continue;
        }
        if (const auto dropped = input.takeDroppedPath()) {
            tryLoadImageMap(*dropped);
        }
        if (input.takeOpenImageRequest()) {
            const auto picked = openImageFileDialog(std::filesystem::path(ASSET_DIR) / "maps");
            previousTime = glfwGetTime();
            if (picked) {
                tryLoadImageMap(*picked);
            }
        }
        const double now = glfwGetTime();
        const double frameTime = now - previousTime;
        const double dt = std::min(frameTime, config::kMaxFrameSeconds);
        fpsTime += frameTime;
        ++frameCount;
        if (fpsTime >= 0.5) {
            fps = static_cast<int>(frameCount / fpsTime + 0.5);
            frameCount = 0;
            fpsTime = 0.0;
        }
        previousTime = now;
        const bool start = input.takeStart();
        const bool mapChanged = input.takeMapChanged();
        if (start || mapChanged) {
            loadMap(input.selectedMap());
        }
        if (input.screen() == GameScreen::Playing) {
            camera.look(input.mouseDelta());
            const float speed = input.running() ? config::kRunSpeed : config::kWalkSpeed;
            const glm::vec3 velocity = camera.groundDirection(input.movement()) * speed;
            physicsTime += dt;
            // Chỉ lấy cạnh nhấn Space khi thực sự chạy một bước. Khung hình ngắn vẫn giữ được phím.
            while (physicsTime >= config::kPhysicsStep) {
                const glm::vec3 before = player.eyePosition();
                player.step(velocity, input.takeJump(), level.solids());
                camera.setPosition(player.eyePosition());
                const glm::vec3 delta = camera.position() - before;
                const float walked = player.grounded() ? glm::length(glm::vec2{delta.x, delta.z}) : 0.0f;
                const bool fired = weapon.step(config::kPhysicsStep, input.takeTrigger(), input.takeReload());
                effects.advance(static_cast<float>(config::kPhysicsStep), walked, weapon);
                if (fired) {
                    // Lấy tia tại tâm nhìn trước khi giật camera. Chỉ giao điểm gần nhất nhận sát thương.
                    const glm::vec3 direction = weapon.shotDirection(camera.front());
                    const auto hit = waves.hitScan(level, camera.position(), direction, weapon.damage());
                    const glm::vec3 end = hit ? hit->point : camera.position() + direction * config::kShotRange;
                    camera.applyRecoil(config::kKickDegrees);
                    effects.recordShot(camera, end, hit);
                }
                physicsTime -= config::kPhysicsStep;
                if (waves.step(static_cast<float>(config::kPhysicsStep), camera.position(), level)) {
                    weapon.refillReserve();
                }
                if (waves.health() == 0) {
                    input.setScreen(GameScreen::GameOver);
                    physicsTime = 0;
                    break;
                }
            }
        } else {
            physicsTime = 0.0;
        }
        // Nền sáng dần lúc mở cửa sổ. dt tính bằng giây để thời gian chuyển màu độc lập với FPS.
        elapsed = std::min(elapsed + dt, config::kFadeSeconds);
        const float brightness = static_cast<float>(elapsed / config::kFadeSeconds);
        const glm::vec3 sky = brightness * level.theme().skyHorizon;
        glClearColor(sky.r, sky.g, sky.b, 1.0f);
        // Xóa màu để đặt nền mới và xóa độ sâu để giá trị từ khung trước
        // không làm mất các fragment ở khung hiện tại khi kiểm tra độ sâu.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int width, height;
        glfwGetFramebufferSize(handle, &width, &height);
        if (width > 0 && height > 0) {
            // Tỉ lệ pixel thực giữ đúng phối cảnh khi kéo cạnh cửa sổ.
            const float aspect = static_cast<float>(width) / static_cast<float>(height);
            renderer.draw(level, camera, aspect, input.flashlightOn(), brightness, effects.flashPosition(),
                          waves.enemies(), input.shadowsOn());
            effects.draw(camera, aspect, input.sparksOn());
            hud.draw(width, height, weapon, waves, effects.hitMarker(), input.screen(),
                     input.showFps() ? fps : -1, input.selectedMap(), &customView);
        }
        GL_CHECK();
        glfwSwapBuffers(handle);
    }
}
