#pragma once

#include <glm/vec3.hpp>
#include <optional>
#include <vector>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

// localBody được mô tả tương đối với origin. Trả về true nếu chạm mặt trên trục đang quét.
bool sweepAxis(glm::vec3& origin, const AABB& localBody, int axis, float distance,
               const std::vector<AABB>& solids);

// Tia origin + t * direction, t >= 0. Nếu gốc nằm trong hộp, t đầu tiên bằng 0.
std::optional<float> rayVsAABB(glm::vec3 origin, glm::vec3 direction, const AABB& box,
                               glm::vec3* entryNormal = nullptr);
