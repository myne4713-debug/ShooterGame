#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    // Gọi use() trước khi đặt uniform vì OpenGL 3.3 ghi vào program đang hoạt động.
    void setMat4(const std::string& name, const glm::mat4& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setFloat(const std::string& name, float value);
    void setInt(const std::string& name, int value);
    void setIntArray(const std::string& name, const GLint* values, GLsizei count);

private:
    Shader() = default;
    GLint uniformLocation(const std::string& name);
    GLuint m_program = 0;
    std::unordered_map<std::string, GLint> m_locations;
};
