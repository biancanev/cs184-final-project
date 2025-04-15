#include "texture.h"
    
bool Texture::loadTexture(const std::string& path){
    Assimp::Importer importer;
    
    // Load the image file with Assimp
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs);
    
    if (!scene || !scene->HasTextures() || !scene->mTextures[0]) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    // Get the first texture from the scene
    aiTexture* texture = scene->mTextures[0];
    
    // Check if the texture data is available
    if (!texture->pcData) {
        std::cerr << "Texture data is not available." << std::endl;
        return false;
    }
    
    // Get texture dimensions and format
    width = texture->mWidth;
    height = texture->mHeight;
    
    // Generate OpenGL texture
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Determine format based on channels
    GLenum format = GL_RGB; // Default
    if (texture->achFormatHint[0] == 'r' && texture->achFormatHint[1] == 'g' && 
        texture->achFormatHint[2] == 'b' && texture->achFormatHint[3] == 'a') {
        format = GL_RGBA;
        channels = 4;
    } else {
        channels = 3;
    }
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture->pcData);
    glGenerateMipmap(GL_TEXTURE_2D);
    
        std::cout << "Texture loaded with Assimp: " << path << " (" << width << "x" << height << ")" << std::endl;
        return true;
}

void Texture::bind(unsigned int slot) const{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() const{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::cleanup(){
    if (ID != 0) {
        glDeleteTextures(1, &ID);
        ID = 0;
    }
}