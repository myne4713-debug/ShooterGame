#include "physics/AABB.h"

#include <algorithm>
#include <limits>

bool sweepAxis(glm::vec3& origin, const AABB& localBody, int axis, float distance,
               const std::vector<AABB>& solids) {
    if (distance == 0.0f) {
        return false;
    }
    const int a = (axis + 1) % 3;
    const int b = (axis + 2) % 3;
    const float start = origin[axis];
    float end = start + distance;
    bool hit = false;
    for (const AABB& box : solids) {
        // Nới vật cản bằng kích thước thân. Giờ chỉ cần quét một điểm từ start đến end.
        const glm::vec3 low = box.min - localBody.max;
        const glm::vec3 high = box.max - localBody.min;
        // Hai hình chỉ chạm mép ở trục còn lại vẫn trượt được dọc mặt đó.
        if (origin[a] <= low[a] || origin[a] >= high[a] ||
            origin[b] <= low[b] || origin[b] >= high[b]) {
            continue;
        }
        if (distance > 0 && start <= low[axis] && end >= low[axis]) {
            end = low[axis];
            hit = true;
        } else if (distance < 0 && start >= high[axis] && end <= high[axis]) {
            end = high[axis];
            hit = true;
        }
    }
    origin[axis] = end;
    return hit;
}

std::optional<float> rayVsAABB(glm::vec3 origin, glm::vec3 direction, const AABB& box,
                               glm::vec3* entryNormal) {
    if (direction == glm::vec3{0.0f}) {
        return std::nullopt;
    }
    float enter = 0.0f;
    glm::vec3 normal{0.0f};
    float leave = std::numeric_limits<float>::infinity();
    for (int axis = 0; axis < 3; ++axis) {
        if (direction[axis] == 0.0f) {
            if (origin[axis] < box.min[axis] || origin[axis] > box.max[axis]) {
                return std::nullopt;
            }
            continue;
        }
        // Mỗi cặp mặt giới hạn một khoảng t. Chỉ phần giao của cả ba khoảng nằm trong hộp.
        float nearT = (box.min[axis] - origin[axis]) / direction[axis];
        float farT = (box.max[axis] - origin[axis]) / direction[axis];
        if (nearT > farT) {
            std::swap(nearT, farT);
        }
        if (nearT >= enter) {
            enter = nearT;
            normal = glm::vec3{0.0f};
            normal[axis] = direction[axis] > 0.0f ? -1.0f : 1.0f;
        }
        leave = std::min(leave, farT);
        if (enter > leave) {
            return std::nullopt;
        }
    }
    if (entryNormal) {
        *entryNormal = normal;
    }
    return enter;
}
