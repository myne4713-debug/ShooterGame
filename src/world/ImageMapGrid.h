#pragma once

#include <glm/vec3.hpp>
#include <string>
#include <utility>
#include <vector>

namespace imagegrid {
struct ImageCell {
    glm::vec3 rgb{0.5f};
    float lum = 0.5f;
    float score = 0.0f;
    int tier = 0; // 0 = trống, 1 = thùng thấp, 2 = khối vừa, 3 = cột cao.
};

struct RectBlock {
    int gx, gz, w, h, tier;
    glm::vec3 color;
};

float luminance(glm::vec3 c);
float saturation(glm::vec3 c);
std::string sanitizeHudLabel(const std::string& raw);
glm::vec3 cellWorldCenter(int gx, int gz, int gridW, int gridH);
void assignTiersFromImage(std::vector<ImageCell>& cells, int gridW, int gridH);
std::pair<std::vector<bool>, int> largestEmptyComponent(const std::vector<ImageCell>& cells, int w, int h);
bool staysConnectedWithout(const std::vector<bool>& inMain, int w, int h, int skipIdx);
std::vector<RectBlock> mergeRectangles(const std::vector<ImageCell>& cells, int w, int h, bool matchColor);
}
