#pragma once

#include "world/MapCatalog.h"
#include <cstdint>
#include <filesystem>
#include <string>

struct ImageMapResult {
    MapDefinition definition;
    int gridWidth = 0;
    int gridHeight = 0;
    int blockCount = 0;
};

ImageMapResult buildMapFromPixels(int width, int height, const unsigned char* rgbaPixels,
                                    const std::string& label, std::uint64_t revision = 1);

ImageMapResult loadMapFromImageFile(const std::filesystem::path& path, std::uint64_t revision = 1);
