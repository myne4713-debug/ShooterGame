#include "gfx/Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
std::string infoLog(GLuint object, bool isProgram) {
    GLint length = 0;
    if (isProgram) {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
    } else {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
    }
    std::string log(static_cast<std::size_t>(std::max(length, 1)), '\0');
    GLsizei written = 0;
    if (isProgram) {
        glGetProgramInfoLog(object, length, &written, log.data());
    } else {
        glGetShaderInfoLog(object, length, &written, log.data());
    }
    log.resize(static_cast<std::size_t>(written));
    return log;
}

GLuint compileStage(GLuint program, GLenum type, const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open shader: " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    const std::string source = contents.str();
    const char* text = source.c_str();
    const GLuint stage = glCreateShader(type);
    if (!stage) {
        throw std::runtime_error("Cannot create shader: " + path.string());
    }
    // Program giữ stage sau khi attach. Đánh dấu xóa ngay để cả nhánh lỗi cũng thu hồi được.
    glAttachShader(program, stage);
    glDeleteShader(stage);
    glShaderSource(stage, 1, &text, nullptr);
    glCompileShader(stage);
    GLint compiled = GL_FALSE;
    glGetShaderiv(stage, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        throw std::runtime_error("Compile failed: " + path.string() + "\n" + infoLog(stage, false));
    }
    return stage;
}
}

Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
    : Shader() {
    m_program = glCreateProgram();
    const std::string paths = vertexPath.string() + " + " + fragmentPath.string();
    if (!m_program) {
        throw std::runtime_error("Cannot create program: " + paths);
    }
    const GLuint vertex = compileStage(m_program, GL_VERTEX_SHADER, vertexPath);
    const GLuint fragment = compileStage(m_program, GL_FRAGMENT_SHADER, fragmentPath);
    glLinkProgram(m_program);
    GLint linked = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        throw std::runtime_error("Link failed: " + paths + "\n" + infoLog(m_program, true));
    }
    glDetachShader(m_program, vertex);
    glDetachShader(m_program, fragment);
}

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept : Shader() {
    *this = std::move(other);
}

Shader& Shader::operator=(Shader&& other) noexcept {
    std::swap(m_program, other.m_program);
    m_locations.swap(other.m_locations);
    return *this;
}

void Shader::use() const {
    glUseProgram(m_program);
}

GLint Shader::uniformLocation(const std::string& name) {
    const auto found = m_locations.find(name);
    if (found != m_locations.end()) {
        return found->second;
    }
    const GLint location = glGetUniformLocation(m_program, name.c_str());
    m_locations.emplace(name, location);
    return location;
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(uniformLocation(name), value);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(uniformLocation(name), value);
}

void Shader::setIntArray(const std::string& name, const GLint* values, GLsizei count) {
    glUniform1iv(uniformLocation(name), count, values);
}
