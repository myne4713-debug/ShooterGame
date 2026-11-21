#include "world/ImageMapGrid.h"
#include "core/Config.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <array>
#include <numeric>
#include <queue>

namespace imagegrid {
namespace {
constexpr float kCellSize = config::kFloorTileSize;

// Tính ngưỡng Otsu phân tách vùng sáng/tối trên biểu đồ tần suất độ sáng 64 mức.
float computeOtsuThreshold(const std::vector<ImageCell>& cells) {
    constexpr int kBins = 64;
    std::array<int, kBins> hist{};
    for (const auto& cell : cells) {
        const int b = std::clamp(static_cast<int>(cell.lum * (kBins - 1)), 0, kBins - 1);
        ++hist[b];
    }
    const float total = static_cast<float>(cells.size());
    float sumAll = 0.0f;
    for (int i = 0; i < kBins; ++i) sumAll += static_cast<float>(i * hist[i]);

    float sumBg = 0.0f, wBg = 0.0f, bestVar = -1.0f;
    int bestBin = kBins / 2;
    for (int t = 0; t < kBins - 1; ++t) {
        wBg += static_cast<float>(hist[t]);
        if (wBg <= 0.0f) continue;
        const float wFg = total - wBg;
        if (wFg <= 0.0f) break;
        sumBg += static_cast<float>(t * hist[t]);
        const float meanBg = sumBg / wBg;
        const float meanFg = (sumAll - sumBg) / wFg;
        const float var = wBg * wFg * (meanBg - meanFg) * (meanBg - meanFg);
        if (var > bestVar) {
            bestVar = var;
            bestBin = t;
        }
    }
    return (static_cast<float>(bestBin) + 0.5f) / static_cast<float>(kBins);
}
}

float luminance(glm::vec3 c) {
    return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

float saturation(glm::vec3 c) {
    const float hi = std::max({c.r, c.g, c.b});
    const float lo = std::min({c.r, c.g, c.b});
    return hi > 0.001f ? (hi - lo) / hi : 0.0f;
}

std::string sanitizeHudLabel(const std::string& raw) {
    std::string out;
    bool prevSpace = true;
    for (unsigned char ch : raw) {
        if (ch >= 'a' && ch <= 'z') ch = static_cast<unsigned char>(ch - 'a' + 'A');
        const bool ok = (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-';
        if (ok) {
            out.push_back(static_cast<char>(ch));
            prevSpace = false;
        } else if ((ch == ' ' || ch == '_' || ch == '.') && !prevSpace && !out.empty()) {
            out.push_back(' ');
            prevSpace = true;
        }
        if (out.size() >= 14) break;
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out.empty() ? "IMAGE MAP" : out;
}

glm::vec3 cellWorldCenter(int gx, int gz, int gridW, int gridH) {
    return {(static_cast<float>(gx) + 0.5f - static_cast<float>(gridW) * 0.5f) * kCellSize,
            0.0f,
            (static_cast<float>(gz) + 0.5f - static_cast<float>(gridH) * 0.5f) * kCellSize};
}

void assignTiersFromImage(std::vector<ImageCell>& cells, int gridW, int gridH) {
    float minLum = 1.0f, maxLum = 0.0f;
    for (const auto& c : cells) {
        minLum = std::min(minLum, c.lum);
        maxLum = std::max(maxLum, c.lum);
    }
    if (maxLum - minLum < 0.08f) {
        // Ảnh đồng màu: tạo 4 trụ che đối xứng để đấu trường vẫn có chiều sâu chiến thuật.
        const int x1 = gridW / 3, x2 = (2 * gridW) / 3;
        const int z1 = gridH / 3, z2 = (2 * gridH) / 3;
        cells[z1 * gridW + x1].tier = 1;
        cells[z1 * gridW + x2].tier = 2;
        cells[z2 * gridW + x1].tier = 2;
        cells[z2 * gridW + x2].tier = 3;
        return;
    }

    const float otsuT = computeOtsuThreshold(cells);
    int darkCount = 0;
    for (const auto& c : cells) {
        if (c.lum <= otsuT) ++darkCount;
    }
    const bool darkObstacles = darkCount <= static_cast<int>(cells.size() * 0.52f);
    std::vector<int> order(cells.size());
    std::iota(order.begin(), order.end(), 0);
    int positiveCount = 0;
    for (auto& c : cells) {
        c.score = darkObstacles ? (otsuT - c.lum) : (c.lum - otsuT);
        if (c.score > 0.0f) ++positiveCount;
    }
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return cells[a].score > cells[b].score;
    });

    const int total = static_cast<int>(cells.size());
    const int minSolid = std::max(4, total / 10);
    const int maxSolid = std::max(minSolid, (total * 36) / 100);
    const int targetSolid = std::clamp(positiveCount, minSolid, maxSolid);

    const float loScore = cells[order[targetSolid - 1]].score;
    const float hiScore = cells[order[0]].score;
    const float span = hiScore - loScore;
    for (int rank = 0; rank < targetSolid; ++rank) {
        const int idx = order[rank];
        const int gx = idx % gridW, gz = idx / gridW;
        const float h = span > 0.02f
            ? (cells[idx].score - loScore) / span
            : static_cast<float>((gx + gz) % 3) * 0.4f;
        cells[idx].tier = h < 0.30f ? 1 : (h < 0.65f ? 2 : 3);
    }
}

std::pair<std::vector<bool>, int> largestEmptyComponent(const std::vector<ImageCell>& cells, int w, int h) {
    const int n = w * h;
    std::vector<int> label(n, -1);
    std::vector<int> sizes;
    std::queue<int> q;
    constexpr int dx[]{1, -1, 0, 0};
    constexpr int dz[]{0, 0, 1, -1};
    for (int i = 0; i < n; ++i) {
        if (cells[i].tier != 0 || label[i] != -1) continue;
        const int id = static_cast<int>(sizes.size());
        int count = 0;
        label[i] = id;
        q.push(i);
        while (!q.empty()) {
            const int cur = q.front();
            q.pop();
            ++count;
            const int cx = cur % w, cz = cur / w;
            for (int d = 0; d < 4; ++d) {
                const int nx = cx + dx[d], nz = cz + dz[d];
                if (nx < 0 || nx >= w || nz < 0 || nz >= h) continue;
                const int ni = nz * w + nx;
                if (cells[ni].tier == 0 && label[ni] == -1) {
                    label[ni] = id;
                    q.push(ni);
                }
            }
        }
        sizes.push_back(count);
    }
    std::vector<bool> inMain(n, false);
    if (sizes.empty()) return {inMain, 0};
    const int bestId = static_cast<int>(std::max_element(sizes.begin(), sizes.end()) - sizes.begin());
    for (int i = 0; i < n; ++i) inMain[i] = label[i] == bestId;
    return {inMain, sizes[bestId]};
}

bool staysConnectedWithout(const std::vector<bool>& inMain, int w, int h, int skipIdx) {
    int start = -1, expected = 0;
    for (int i = 0; i < w * h; ++i) {
        if (!inMain[i] || i == skipIdx) continue;
        if (start == -1) start = i;
        ++expected;
    }
    if (expected <= 1) return false;
    std::vector<bool> seen(w * h, false);
    std::queue<int> q;
    seen[start] = true;
    q.push(start);
    int visited = 0;
    constexpr int dx[]{1, -1, 0, 0};
    constexpr int dz[]{0, 0, 1, -1};
    while (!q.empty()) {
        const int cur = q.front();
        q.pop();
        ++visited;
        const int cx = cur % w, cz = cur / w;
        for (int d = 0; d < 4; ++d) {
            const int nx = cx + dx[d], nz = cz + dz[d];
            if (nx < 0 || nx >= w || nz < 0 || nz >= h) continue;
            const int ni = nz * w + nx;
            if (inMain[ni] && ni != skipIdx && !seen[ni]) {
                seen[ni] = true;
                q.push(ni);
            }
        }
    }
    return visited == expected;
}

std::vector<RectBlock> mergeRectangles(const std::vector<ImageCell>& cells, int w, int h, bool matchColor) {
    std::vector<bool> used(w * h, false);
    std::vector<RectBlock> blocks;
    const auto canMerge = [&](int idx, const ImageCell& base) {
        if (used[idx] || cells[idx].tier != base.tier) return false;
        return !matchColor || glm::distance(cells[idx].rgb, base.rgb) <= 0.28f;
    };
    for (int gz = 0; gz < h; ++gz) {
        for (int gx = 0; gx < w; ++gx) {
            const int start = gz * w + gx;
            if (used[start] || cells[start].tier == 0) continue;
            const ImageCell& base = cells[start];
            int rw = 1, rh = 1;
            while (gx + rw < w && canMerge(gz * w + gx + rw, base)) ++rw;
            while (gz + rh < h) {
                bool rowOk = true;
                for (int dx = 0; dx < rw; ++dx) {
                    if (!canMerge((gz + rh) * w + gx + dx, base)) {
                        rowOk = false;
                        break;
                    }
                }
                if (!rowOk) break;
                ++rh;
            }
            glm::vec3 sumColor{0.0f};
            for (int dz = 0; dz < rh; ++dz) {
                for (int dx = 0; dx < rw; ++dx) {
                    const int idx = (gz + dz) * w + gx + dx;
                    used[idx] = true;
                    sumColor += cells[idx].rgb;
                }
            }
            blocks.push_back({gx, gz, rw, rh, base.tier, sumColor / static_cast<float>(rw * rh)});
        }
    }
    return blocks;
}
}
