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
    VBO(const std::vector<Vertex>& vertices);
    VBO(const std::vector<float>& vertices);
    ~VBO();

    void bind() const;
    void unbind() const;
    void update(const std::vector<float>& vertices);
};

class EBO {
public:
    unsigned int ID;
    EBO(const std::vector<unsigned int>& indices);
    ~EBO();

    void bind() const;
    void unbind() const;
};

class VAO {
public:
    unsigned int ID;
    VAO();
    ~VAO();

    void linkAttrib(VBO& VBO, unsigned int layout, unsigned int numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void bind() const;
    void unbind() const;
};

