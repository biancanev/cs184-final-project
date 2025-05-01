#ifndef TEXTURE_H
#define TEXTURE_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <GL/glew.h>
#include <string>
#include <iostream>

class Texture{
    public:
        unsigned int id;
        std::string type;
        int width, height, channels;
        
        Texture() : id(0), width(0), height(0), channels(0) {}
        
        bool loadTexture(const std::string& path);

        void bind(unsigned int slot = 0) const;

        void unbind() const;

        void cleanup();

        bool loadTextureFromFile(const std::string& path);
};


#endif