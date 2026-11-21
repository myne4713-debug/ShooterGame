#pragma once

#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> openImageFileDialog(const std::filesystem::path& initialDir);
