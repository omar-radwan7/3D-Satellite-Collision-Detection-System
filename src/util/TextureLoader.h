#pragma once
#include <string>
#include <GL/glew.h>

class TextureLoader {
public:
    static unsigned int LoadTexture(const std::string& path);
};

