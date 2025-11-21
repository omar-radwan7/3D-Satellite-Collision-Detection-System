#include "Buffers.h"

VBO::VBO(std::vector<Vertex>& vertices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }
void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void VBO::remove() { glDeleteBuffers(1, &ID); }

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::linkAttrib(VBO& VBO, unsigned int layout, unsigned int numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO.bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO.unbind();
}

void VAO::bind() { glBindVertexArray(ID); }
void VAO::unbind() { glBindVertexArray(0); }
void VAO::remove() { glDeleteVertexArrays(1, &ID); }

EBO::EBO(std::vector<unsigned int>& indices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void EBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
void EBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
void EBO::remove() { glDeleteBuffers(1, &ID); }
