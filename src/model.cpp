#include "model.h"

Model::Model(const std::string &path, bool gamma) : gammaCorrection(gamma) {
    loadModel(path);
}

// Default constructor creates a simple colored cube
Model::Model() {
    createCube();
}

void Model::Draw(Shader &shader) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(const std::string &path) {
    // Read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
    // Check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    
    // Retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // Process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    // Process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node object only contains indices to index the actual objects in the scene. 
        // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    
    // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    // Data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        
        // Positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        // Normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        // Texture Coordinates
        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            // Tangent
            if (mesh->HasTangentsAndBitangents()) {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                
                // Bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    
    // Now walk through each of the mesh's faces and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }
    
    // Process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    
    // For simplicity without texture loading, we'll just use diffuse color from material
    // and skip actual texture loading
    aiColor3D color(1.0f, 1.0f, 1.0f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    
    // We'll keep the textures vector structure but won't actually load textures
    // This will make it easier to add texture support later
    
    // Return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    // For now, we'll return an empty vector since we're not loading textures
    return std::vector<Texture>();
}

// Create a simple cube model for testing
void Model::createCube() {
    // Cube vertices
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Define the 8 vertices of a cube
    Vertex vertex;
    
    // Front face (z = 0.5)
    vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Back face (z = -0.5)
    vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Define the 12 triangles of the cube (2 per face)
    // Front face
    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    indices.push_back(2); indices.push_back(3); indices.push_back(0);
    
    // Right face
    indices.push_back(1); indices.push_back(5); indices.push_back(6);
    indices.push_back(6); indices.push_back(2); indices.push_back(1);
    
    // Back face
    indices.push_back(5); indices.push_back(4); indices.push_back(7);
    indices.push_back(7); indices.push_back(6); indices.push_back(5);
    
    // Left face
    indices.push_back(4); indices.push_back(0); indices.push_back(3);
    indices.push_back(3); indices.push_back(7); indices.push_back(4);
    
    // Top face
    indices.push_back(3); indices.push_back(2); indices.push_back(6);
    indices.push_back(6); indices.push_back(7); indices.push_back(3);
    
    // Bottom face
    indices.push_back(4); indices.push_back(5); indices.push_back(1);
    indices.push_back(1); indices.push_back(0); indices.push_back(4);
    
    // Create a mesh with these vertices and indices
    std::vector<Texture> textures; // Empty for now
    meshes.push_back(Mesh(vertices, indices, textures));
}