#pragma once

#ifdef SHOOTER_DEBUG
#include <glad/glad.h>
#include <cstdio>

inline void checkGlErrors(const char* file, int line) {
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        std::fprintf(stderr, "OpenGL error 0x%04X at %s:%d\n", error, file, line);
    }
}
#define GL_CHECK() checkGlErrors(__FILE__, __LINE__)
#else
#define GL_CHECK() ((void)0)
#endif
