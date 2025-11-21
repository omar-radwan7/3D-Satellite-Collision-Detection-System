#pragma once
#include <GL/glew.h>
#include <vector>

struct Vertex {
    float position[3];
    float normal[3];
    float texCoords[2];
};

class VBO {
public:
    unsigned int ID;
    VBO(std::vector<Vertex>& vertices);
    void bind();
    void unbind();
    void remove();
};

class VAO {
public:
    unsigned int ID;
    VAO();
    void linkAttrib(VBO& VBO, unsigned int layout, unsigned int numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void bind();
    void unbind();
    void remove();
};

class EBO {
public:
    unsigned int ID;
    EBO(std::vector<unsigned int>& indices);
    void bind();
    void unbind();
    void remove();
};
