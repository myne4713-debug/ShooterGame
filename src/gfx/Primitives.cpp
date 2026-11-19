#include "gfx/Primitives.h"
#include "core/Config.h"

#include <glm/gtc/constants.hpp>
#include <cmath>

namespace {
using Vertices = std::vector<Vertex>;
using Indices = std::vector<GLuint>;

// Hai cạnh u, v theo thứ tự ngược chiều kim đồng hồ khi nhìn từ ngoài.
// Mỗi mặt có bốn đỉnh riêng để cùng vị trí ở cạnh vẫn có pháp tuyến và UV riêng.
void addFace(Vertices& vertices, Indices& indices, glm::vec3 center,
             glm::vec3 u, glm::vec3 v, float tiling = 1.0f) {
    const auto first = static_cast<GLuint>(vertices.size());
    const glm::vec3 origin = center - (u + v) * 0.5f;
    const glm::vec3 normal = glm::normalize(glm::cross(u, v));
    vertices.insert(vertices.end(), {
        {origin, normal, {0, 0}}, {origin + u, normal, {tiling, 0}},
        {origin + u + v, normal, {tiling, tiling}}, {origin + v, normal, {0, tiling}}
    });
    indices.insert(indices.end(), {first, first+1, first+2, first, first+2, first+3});
}

glm::vec3 ringDirection(unsigned int slice) {
    // Lặp đúng điểm đầu ở đường nối, với u = 1 ở bản sao để UV không bị kéo ngược.
    const float angle = glm::two_pi<float>() * (slice % config::kSlices) / config::kSlices;
    return {std::cos(angle), 0.0f, std::sin(angle)};
}
}

Mesh makeCube() {
    Vertices vertices;
    Indices indices;
    const glm::vec3 x{1, 0, 0}, y{0, 1, 0}, z{0, 0, 1};
    addFace(vertices, indices,  z * 0.5f,  x, y);
    addFace(vertices, indices, -z * 0.5f, -x, y);
    addFace(vertices, indices,  x * 0.5f, -z, y);
    addFace(vertices, indices, -x * 0.5f,  z, y);
    addFace(vertices, indices,  y * 0.5f,  x, -z);
    addFace(vertices, indices, -y * 0.5f,  x,  z);
    return Mesh(vertices, indices);
}

Mesh makePlane(float tiling) {
    Vertices vertices;
    Indices indices;
    addFace(vertices, indices, {0, 0, 0}, {1, 0, 0}, {0, 0, -1}, tiling);
    return Mesh(vertices, indices);
}

Mesh makeQuad() {
    Vertices vertices;
    Indices indices;
    addFace(vertices, indices, {0, 0, 0}, {1, 0, 0}, {0, 1, 0});
    return Mesh(vertices, indices);
}

Mesh makeSphere() {
    Vertices vertices;
    Indices indices;
    for (unsigned int row = 0; row <= config::kStacks; ++row) {
        const float latitude = static_cast<float>(row) / config::kStacks;
        const float theta = latitude * glm::pi<float>();
        for (unsigned int col = 0; col <= config::kSlices; ++col) {
            glm::vec3 normal = std::sin(theta) * ringDirection(col);
            normal.y = std::cos(theta);
            if (row == 0 || row == config::kStacks) {
                normal = {0, row == 0 ? 1.0f : -1.0f, 0};
            }
            vertices.push_back({0.5f * normal, normal,
                                {static_cast<float>(col) / config::kSlices, 1.0f - latitude}});
        }
    }
    for (GLuint row = 0; row < config::kStacks; ++row) {
        for (GLuint col = 0; col < config::kSlices; ++col) {
            const GLuint top = row * (config::kSlices + 1) + col;
            const GLuint bottom = top + config::kSlices + 1;
            // Ở mỗi cực, hai đỉnh cùng vĩ tuyến trùng nhau nên chỉ tạo một tam giác.
            if (row > 0) {
                indices.insert(indices.end(), {top, top+1, bottom+1});
            }
            if (row + 1 < config::kStacks) {
                indices.insert(indices.end(), {top, bottom+1, bottom});
            }
        }
    }
    return Mesh(vertices, indices);
}

Mesh makeCylinder() {
    Vertices vertices;
    Indices indices;
    for (GLuint col = 0; col <= config::kSlices; ++col) {
        const glm::vec3 normal = ringDirection(col);
        const glm::vec3 radius = 0.5f * normal;
        const float u = static_cast<float>(col) / config::kSlices;
        vertices.push_back({{radius.x, -0.5f, radius.z}, normal, {u, 0}});
        vertices.push_back({{radius.x,  0.5f, radius.z}, normal, {u, 1}});
        if (col < config::kSlices) {
            const GLuint base = 2 * col;
            indices.insert(indices.end(), {base, base+1, base+3, base, base+3, base+2});
        }
    }
    // Đỉnh vành nắp tách khỏi thân để nắp có pháp tuyến thẳng đứng và UV phẳng.
    for (float sign : {-1.0f, 1.0f}) {
        const auto center = static_cast<GLuint>(vertices.size());
        const glm::vec3 normal{0, sign, 0};
        vertices.push_back({0.5f * normal, normal, {0.5f, 0.5f}});
        for (GLuint col = 0; col <= config::kSlices; ++col) {
            const glm::vec3 radius = 0.5f * ringDirection(col);
            vertices.push_back({radius + 0.5f * normal, normal,
                                {0.5f + radius.x, 0.5f - sign * radius.z}});
            if (col > 0) {
                const GLuint previous = center + col;
                if (sign > 0) {
                    indices.insert(indices.end(), {center, previous+1, previous});
                } else {
                    indices.insert(indices.end(), {center, previous, previous+1});
                }
            }
        }
    }
    return Mesh(vertices, indices);
}
