#include "core/Game.h"
#include "core/Window.h"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <optional>

int main(int argc, char** argv) {
    glfwSetErrorCallback([](int code, const char* description) {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
    });
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;
    try {
        std::optional<std::filesystem::path> initialImage;
        if (argc >= 2 && argv[1] && argv[1][0] != '\0') {
            initialImage = std::filesystem::path(argv[1]);
        }
        Window window;
        runGame(window, initialImage);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        status = EXIT_FAILURE;
    }
    glfwTerminate();
    return status;
}
