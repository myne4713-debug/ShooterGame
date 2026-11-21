#pragma once

#include <filesystem>
#include <optional>

class Window;

void runGame(Window& window, const std::optional<std::filesystem::path>& initialImage = std::nullopt);
