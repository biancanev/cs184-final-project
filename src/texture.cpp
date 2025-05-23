#include "texture.h"
#include "stb_image.h"
    
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
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
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
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::cleanup(){
    if (id != 0) {
        glDeleteTextures(1, &id);
        id = 0;
    }
}

bool Texture::loadTextureFromFile(const std::string& path) {
    // Generate OpenGL texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Store texture properties
        this->width = width;
        this->height = height;
        this->channels = nrChannels;
        this->type = "texture_diffuse";

        stbi_image_free(data);
        return true;
    }
    
    std::cerr << "Failed to load texture: " << path << std::endl;
    return false;
}
