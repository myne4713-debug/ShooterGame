#include "core/Game.h"
#include "core/Window.h"

#include <cstdio>
#include <cstdlib>
#include <exception>

int main() {
    glfwSetErrorCallback([](int code, const char* description) {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
    });
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;
    try {
        Window window;
        runGame(window);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        status = EXIT_FAILURE;
    }
    glfwTerminate();
    return status;
}
