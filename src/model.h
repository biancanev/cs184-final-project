// model.h
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

// Forward declaration
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

class Model {
public:
    // Model data 
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    // Constructor for loading model from file
    Model(const std::string &path, bool gamma = false);
    
    // Constructor for creating a default cube
    Model();

    // Draws the model, and thus all its meshes
    void Draw(Shader &shader);

    void createGrid(float size, int subdivisions);
    void replaceTextures(const std::vector<Texture>& newTextures);
    
private:
    // Loads a model with supported ASSIMP extensions from file
    void loadModel(const std::string &path);

    // Processes a node in a recursive fashion
    void processNode(aiNode *node, const aiScene *scene);

    // Process an individual mesh
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // Loads material textures
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    
    // Creates a default cube for testing
    void createCube();

};
#endif