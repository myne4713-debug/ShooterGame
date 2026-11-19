#include "gfx/Mesh.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
    : Mesh() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    // VAO ghi nhớ EBO đang gắn. Hai tam giác kề nhau có thể tra cùng một đỉnh qua chỉ số.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                 indices.data(), GL_STATIC_DRAW);

    // Stride đi từ đỉnh này tới đỉnh kế tiếp. Offset chọn thuộc tính bên trong mỗi đỉnh.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (const GLenum error = glGetError(); error != GL_NO_ERROR) {
        throw std::runtime_error("Mesh upload failed, OpenGL error " + std::to_string(error));
    }
    m_indexCount = static_cast<GLsizei>(indices.size());
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

Mesh::Mesh(Mesh&& other) noexcept : Mesh() {
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    std::swap(m_vao, other.m_vao);
    std::swap(m_vbo, other.m_vbo);
    std::swap(m_ebo, other.m_ebo);
    std::swap(m_indexCount, other.m_indexCount);
    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
}

void Mesh::drawInstanced(GLsizei count) const {
    glBindVertexArray(m_vao);
    glDrawElementsInstanced(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr, count);
}
