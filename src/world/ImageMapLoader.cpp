#include "world/ImageMapLoader.h"
#include "core/Config.h"
#include "world/ImageMapGrid.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
constexpr float kCellSize = config::kFloorTileSize;
constexpr std::size_t kMaxInteriorBoxes = 28;
}

ImageMapResult buildMapFromPixels(int width, int height, const unsigned char* rgbaPixels,
                                    const std::string& label, std::uint64_t revision) {
    if (width <= 0 || height <= 0 || !rgbaPixels) {
        throw std::invalid_argument("Invalid image pixel buffer");
    }

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    int gridW = 14, gridH = 14;
    if (aspect >= 1.0f) {
        gridW = std::clamp(static_cast<int>(std::lround(13.0f * std::min(aspect, 1.35f))), 12, 16);
        gridH = std::clamp(static_cast<int>(std::lround(static_cast<float>(gridW) / aspect)), 10, 16);
    } else {
        gridH = std::clamp(static_cast<int>(std::lround(13.0f * std::min(1.0f / aspect, 1.35f))), 12, 16);
        gridW = std::clamp(static_cast<int>(std::lround(static_cast<float>(gridH) * aspect)), 10, 16);
    }

    // Giảm mẫu có lọc hộp (box filter) từ ảnh gốc sang lưới chiến thuật gridW x gridH.
    std::vector<imagegrid::ImageCell> cells(gridW * gridH);
    glm::vec3 avgColor{0.0f}, topColor{0.0f}, midColor{0.0f};
    int topCount = 0, midCount = 0;
    for (int gz = 0; gz < gridH; ++gz) {
        const int y0 = (gz * height) / gridH;
        const int y1 = std::max(y0 + 1, ((gz + 1) * height) / gridH);
        for (int gx = 0; gx < gridW; ++gx) {
            const int x0 = (gx * width) / gridW;
            const int x1 = std::max(x0 + 1, ((gx + 1) * width) / gridW);
            glm::vec3 sum{0.0f};
            int samples = 0;
            for (int y = y0; y < y1; ++y) {
                for (int x = x0; x < x1; ++x) {
                    const std::size_t off = (static_cast<std::size_t>(y) * width + x) * 4;
                    sum.r += static_cast<float>(rgbaPixels[off]) / 255.0f;
                    sum.g += static_cast<float>(rgbaPixels[off + 1]) / 255.0f;
                    sum.b += static_cast<float>(rgbaPixels[off + 2]) / 255.0f;
                    ++samples;
                }
            }
            const glm::vec3 rgb = sum / static_cast<float>(samples);
            cells[gz * gridW + gx].rgb = rgb;
            cells[gz * gridW + gx].lum = imagegrid::luminance(rgb);
            avgColor += rgb;
            if (gz * 4 < gridH) {
                topColor += rgb;
                ++topCount;
            } else if (gz * 4 < 3 * gridH) {
                midColor += rgb;
                ++midCount;
            }
        }
    }
    avgColor /= static_cast<float>(cells.size());
    topColor = topCount > 0 ? topColor / static_cast<float>(topCount) : avgColor;
    midColor = midCount > 0 ? midColor / static_cast<float>(midCount) : avgColor;

    imagegrid::assignTiersFromImage(cells, gridW, gridH);

    // Đảm bảo vùng trống liên thông 4-hướng đủ rộng cho người chơi và robot dẫn đường.
    auto [inMain, mainCount] = imagegrid::largestEmptyComponent(cells, gridW, gridH);
    if (mainCount < 24) {
        const int cx = gridW / 2, cz = gridH / 2;
        for (int gx = 0; gx < gridW; ++gx) {
            cells[cz * gridW + gx].tier = 0;
            if (cz > 0) cells[(cz - 1) * gridW + gx].tier = 0;
        }
        for (int gz = 0; gz < gridH; ++gz) {
            cells[gz * gridW + cx].tier = 0;
            if (cx > 0) cells[gz * gridW + (cx - 1)].tier = 0;
        }
        std::tie(inMain, mainCount) = imagegrid::largestEmptyComponent(cells, gridW, gridH);
    }
    for (int i = 0; i < gridW * gridH; ++i) {
        if (cells[i].tier == 0 && !inMain[i]) cells[i].tier = 2;
    }

    std::vector<imagegrid::RectBlock> blocks = imagegrid::mergeRectangles(cells, gridW, gridH, true);
    if (blocks.size() > kMaxInteriorBoxes) {
        blocks = imagegrid::mergeRectangles(cells, gridW, gridH, false);
    }
    if (blocks.size() > kMaxInteriorBoxes) {
        std::sort(blocks.begin(), blocks.end(), [](const imagegrid::RectBlock& a, const imagegrid::RectBlock& b) {
            return a.w * a.h < b.w * b.h;
        });
        const std::size_t drop = blocks.size() - kMaxInteriorBoxes;
        for (std::size_t i = 0; i < drop; ++i) {
            for (int dz = 0; dz < blocks[i].h; ++dz) {
                for (int dx = 0; dx < blocks[i].w; ++dx) {
                    cells[(blocks[i].gz + dz) * gridW + blocks[i].gx + dx].tier = 0;
                }
            }
        }
        blocks.erase(blocks.begin(), blocks.begin() + static_cast<std::ptrdiff_t>(drop));
        std::tie(inMain, mainCount) = imagegrid::largestEmptyComponent(cells, gridW, gridH);
    }

    std::vector<LevelBox> interiorBoxes;
    interiorBoxes.reserve(blocks.size());
    for (const auto& b : blocks) {
        const float boxH = b.tier == 1 ? config::kLowCrateHeight : (b.tier == 2 ? 1.80f : 3.20f);
        const Surface surf = b.tier == 1
            ? Surface::Wood
            : (b.tier == 3 && imagegrid::saturation(b.color) > 0.28f ? Surface::Metal : Surface::Wall);
        const float shininess = surf == Surface::Metal
            ? config::kMetalShininess
            : (surf == Surface::Wood ? config::kWoodShininess : config::kWallShininess);
        glm::vec3 col = b.color;
        const float maxC = std::max({col.r, col.g, col.b});
        if (maxC < 0.28f) {
            col = maxC > 0.01f ? col * (0.30f / maxC) : glm::vec3{0.28f, 0.31f, 0.35f};
        }
        col = glm::clamp(col, glm::vec3{0.22f}, glm::vec3{0.95f});
        const float cx = (static_cast<float>(b.gx) + static_cast<float>(b.w) * 0.5f - gridW * 0.5f) * kCellSize;
        const float cz = (static_cast<float>(b.gz) + static_cast<float>(b.h) * 0.5f - gridH * 0.5f) * kCellSize;
        interiorBoxes.push_back({{cx, boxH * 0.5f, cz}, {b.w * kCellSize, boxH, b.h * kCellSize},
                                 {surf, col, shininess}});
    }

    // Đặt người chơi ở nửa Nam trong vùng liên thông, quay mặt về tâm sân.
    int playerIdx = -1;
    int bestPlayerScore = -1000000;
    for (int gz = 0; gz < gridH; ++gz) {
        for (int gx = 0; gx < gridW; ++gx) {
            const int idx = gz * gridW + gx;
            if (!inMain[idx]) continue;
            const bool northOpen = gz > 0 && inMain[(gz - 1) * gridW + gx];
            const int score = gz * 10 - std::abs(gx - gridW / 2) * 3 + (northOpen ? 25 : 0);
            if (score > bestPlayerScore) {
                bestPlayerScore = score;
                playerIdx = idx;
            }
        }
    }
    const glm::vec3 playerSpawn = imagegrid::cellWorldCenter(playerIdx % gridW, playerIdx / gridW, gridW, gridH);
    const glm::vec2 toCenter{-playerSpawn.x, -playerSpawn.z};
    const float playerYaw = glm::length(toCenter) > 0.5f
        ? glm::degrees(std::atan2(toCenter.y, toCenter.x))
        : config::kStartYaw;

    // Đặt bia tập tại ô giữ nguyên tính liên thông của phần còn lại.
    int targetIdx = -1;
    float bestTargetScore = -1e9f;
    for (int i = 0; i < gridW * gridH; ++i) {
        if (!inMain[i] || i == playerIdx || !imagegrid::staysConnectedWithout(inMain, gridW, gridH, i)) continue;
        const glm::vec3 pos = imagegrid::cellWorldCenter(i % gridW, i / gridW, gridW, gridH);
        const float d = glm::distance(pos, playerSpawn);
        const float sc = (d >= 3.5f && d <= 11.0f ? 100.0f : 0.0f) - std::abs(d - 7.0f);
        if (sc > bestTargetScore) {
            bestTargetScore = sc;
            targetIdx = i;
        }
    }
    if (targetIdx < 0) {
        for (int i = 0; i < gridW * gridH; ++i) {
            if (inMain[i] && i != playerIdx) {
                targetIdx = i;
                break;
            }
        }
    }
    const glm::vec3 targetPos = imagegrid::cellWorldCenter(targetIdx % gridW, targetIdx / gridW, gridW, gridH);
    const glm::vec3 targetSize = std::abs(toCenter.x) > std::abs(toCenter.y)
        ? glm::vec3{0.30f, 1.8f, 1.10f}
        : glm::vec3{1.10f, 1.8f, 0.30f};
    const LevelBox targetBox{targetPos + glm::vec3{0.0f, 0.9f, 0.0f}, targetSize,
                             {Surface::Metal, {0.96f, 0.57f, 0.13f}, config::kMetalShininess}};

    // Lấy mẫu điểm xa nhất (farthest-point sampling) cho 12 điểm sinh robot.
    std::vector<int> freeCells;
    for (int i = 0; i < gridW * gridH; ++i) {
        if (inMain[i] && i != playerIdx && i != targetIdx) freeCells.push_back(i);
    }
    std::vector<glm::vec3> spawnPoints;
    std::vector<glm::vec3> refs{playerSpawn};
    std::vector<bool> picked(freeCells.size(), false);
    for (std::size_t k = 0; k < 12 && k < freeCells.size(); ++k) {
        std::size_t bestIdx = 0;
        float bestDist = -1.0f;
        for (std::size_t j = 0; j < freeCells.size(); ++j) {
            if (picked[j]) continue;
            const glm::vec3 p = imagegrid::cellWorldCenter(freeCells[j] % gridW, freeCells[j] / gridW, gridW, gridH);
            float nearest = 1e9f;
            for (const auto& r : refs) nearest = std::min(nearest, glm::distance(p, r));
            if (nearest > bestDist) {
                bestDist = nearest;
                bestIdx = j;
            }
        }
        picked[bestIdx] = true;
        const int cellIdx = freeCells[bestIdx];
        const glm::vec3 pt = imagegrid::cellWorldCenter(cellIdx % gridW, cellIdx / gridW, gridW, gridH);
        spawnPoints.push_back(pt);
        refs.push_back(pt);
    }
    while (spawnPoints.size() < 12 && !spawnPoints.empty()) {
        spawnPoints.push_back(spawnPoints[spawnPoints.size() % freeCells.size()]);
    }

    // Dựng chủ đề bầu trời và ánh sáng từ bảng màu tổng thể của ảnh.
    glm::vec3 floorSum{0.0f};
    int floorCount = 0;
    for (int i = 0; i < gridW * gridH; ++i) {
        if (inMain[i]) {
            floorSum += cells[i].rgb;
            ++floorCount;
        }
    }
    const glm::vec3 floorBase = floorCount > 0 ? floorSum / static_cast<float>(floorCount) : avgColor;
    const float avgLum = imagegrid::luminance(avgColor);
    const glm::vec2 arenaSize{gridW * kCellSize, gridH * kCellSize};
    const glm::vec3 horizon = glm::clamp(glm::mix(midColor, glm::vec3{0.46f, 0.56f, 0.68f}, 0.35f),
                                         glm::vec3{0.10f}, glm::vec3{0.92f});
    const glm::vec3 zenith = glm::clamp(topColor * 0.60f + glm::vec3{0.04f, 0.09f, 0.22f},
                                        glm::vec3{0.02f}, glm::vec3{0.78f});
    const glm::vec3 sunDir = glm::normalize(glm::vec3{-0.52f, avgLum < 0.35f ? -0.95f : -0.78f, -0.35f});
    const glm::vec3 sunCol = glm::mix(glm::vec3{0.42f, 0.52f, 0.72f}, glm::vec3{0.95f, 0.84f, 0.66f},
                                      std::clamp(avgLum * 1.3f, 0.15f, 1.0f));
    const glm::vec3 ambient = glm::mix(glm::vec3{0.18f, 0.21f, 0.29f}, glm::vec3{0.36f, 0.35f, 0.32f},
                                       std::clamp(avgLum, 0.0f, 1.0f));
    const auto lightCol = [&](std::size_t si, glm::vec3 fallback) {
        const glm::vec3 p = spawnPoints[si % spawnPoints.size()];
        const int gx = std::clamp(static_cast<int>(p.x / kCellSize + gridW * 0.5f), 0, gridW - 1);
        const int gz = std::clamp(static_cast<int>(p.z / kCellSize + gridH * 0.5f), 0, gridH - 1);
        const glm::vec3 c = cells[gz * gridW + gx].rgb;
        return glm::mix(fallback, glm::clamp(c * 1.4f, glm::vec3{0.25f}, glm::vec3{1.25f}), 0.55f);
    };
    const MapTheme theme{
        horizon,
        zenith,
        ambient,
        sunDir,
        sunCol,
        10.0f,
        std::max(38.0f, glm::length(arenaSize) + 4.0f),
        0.5f * glm::length(arenaSize) + 2.5f,
        {Surface::Floor, glm::clamp(floorBase, glm::vec3{0.34f}, glm::vec3{0.85f}), 24.0f},
        {{
            {spawnPoints[0] + glm::vec3{0.0f, 3.2f, 0.0f}, lightCol(0, {1.0f, 0.62f, 0.28f})},
            {spawnPoints[1 % spawnPoints.size()] + glm::vec3{0.0f, 3.2f, 0.0f}, lightCol(1, {0.35f, 0.78f, 1.15f})},
            {spawnPoints[2 % spawnPoints.size()] + glm::vec3{0.0f, 3.2f, 0.0f}, lightCol(2, {0.95f, 0.78f, 0.44f})},
        }}};

    const std::string cleanName = imagegrid::sanitizeHudLabel(label);
    const std::string subtitle = "IMAGE " + std::to_string(gridW) + "X" + std::to_string(gridH)
                               + " - " + std::to_string(interiorBoxes.size()) + " BOXES";
    const MapDefinition def{
        MapId::Custom,
        cleanName,
        subtitle,
        revision,
        arenaSize,
        4.2f,
        playerSpawn,
        playerYaw,
        -4.0f,
        theme,
        {Surface::Wall, glm::clamp(glm::mix(avgColor, glm::vec3{0.68f}, 0.45f), glm::vec3{0.32f}, glm::vec3{0.85f}),
         config::kWallShininess},
        std::move(interiorBoxes),
        targetBox,
        std::move(spawnPoints)};
    return {def, gridW, gridH, static_cast<int>(def.interiorBoxes.size())};
}

ImageMapResult loadMapFromImageFile(const std::filesystem::path& path, std::uint64_t revision) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open image file: " + path.string());
    }
    const std::streamsize byteCount = file.tellg();
    if (byteCount <= 0) {
        throw std::runtime_error("Image file is empty: " + path.string());
    }
    std::vector<unsigned char> encoded(static_cast<std::size_t>(byteCount));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), byteCount)) {
        throw std::runtime_error("Failed reading image file: " + path.string());
    }
    // Không lật dọc ảnh bản đồ để hàng trên cùng ứng với phía Bắc (-Z) trước mắt người chơi.
    stbi_set_flip_vertically_on_load(false);
    int width = 0, height = 0;
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels(
        stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()),
                              &width, &height, nullptr, STBI_rgb_alpha),
        stbi_image_free);
    if (!pixels) {
        throw std::runtime_error(std::string("Unsupported or corrupt image: ") + stbi_failure_reason());
    }
    return buildMapFromPixels(width, height, pixels.get(), path.stem().string(), revision);
}
