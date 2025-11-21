#include "TextureLoader.h"
#include <iostream>
#include <vector>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int TextureLoader::LoadTexture(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width = 0, height = 0, nrComponents = 0;
    stbi_set_flip_vertically_on_load(true);

    static const std::vector<std::string> prefixes = {
        "",
        "../",
        "../../"
    };

    unsigned char* data = nullptr;
    std::string resolvedPath;
    for (const auto& prefix : prefixes) {
        std::string fullPath = prefix + path;
        data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            resolvedPath = fullPath;
            break;
        }
    }

    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        else format = GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    return textureID;
}
