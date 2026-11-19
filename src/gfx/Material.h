#pragma once

#include <glm/vec3.hpp>

enum class Surface { Floor, Wall, Wood, Metal };

struct Material {
    Surface surface;
    glm::vec3 color;
    float shininess;
};
