// model.cpp
#include "model.h"

Model::Model(const std::string &path, bool gamma) : gammaCorrection(gamma) {
    loadModel(path);
}

// Default constructor creates a simple colored cube
Model::Model() {
    createCube();
    // createGrid(10, 20);
}

void Model::replaceTextures(const std::vector<Texture>& newTextures) {
    if (!meshes.empty()) {
        meshes[0].textures = newTextures;
    }
}

void Model::Draw(Shader &shader) {
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(const std::string &path) {
    // Read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace);
    
    // Check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
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
        // The node object only contains indices to index the actual objects in the scene
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    
    // After we've processed all of the meshes, recursively process each of the children nodes
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
        
        // Positions
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        
        // Normals
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }
        
        // Texture Coordinates
        if(mesh->mTextureCoords[0]) {
            // A vertex can contain up to 8 different texture coordinates
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x; 
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            
            // Tangent
            if (mesh->HasTangentsAndBitangents()) {
                vertex.Tangent.x = mesh->mTangents[i].x;
                vertex.Tangent.y = mesh->mTangents[i].y;
                vertex.Tangent.z = mesh->mTangents[i].z;
                
                // Bitangent
                vertex.Bitangent.x = mesh->mBitangents[i].x;
                vertex.Bitangent.y = mesh->mBitangents[i].y;
                vertex.Bitangent.z = mesh->mBitangents[i].z;
            } else {
                // Default tangent and bitangent if not provided
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertex.Tangent = glm::vec3(0.0f);
            vertex.Bitangent = glm::vec3(0.0f);
        }

        vertices.push_back(vertex);
    }
    
    // Walk through each of the mesh's faces and retrieve indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }
    
    // Process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    
    // 1. Diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    
    // 2. Specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    
    // 3. Normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    
    // 4. Height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    // Return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    
    // For now, we'll return an empty vector since we're not loading textures yet
    // You can implement actual texture loading here later
    
    // TODO: Implement TextureFromFile function
    
    return textures;
}

void Model::createCube() {
    // Clear any existing meshes
    meshes.clear();
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Define 24 vertices (4 per face, with unique normals)
    Vertex vertex;
    
    // Front face (z = 0.5) - normal = (0, 0, 1)
    vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
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
    
    // Back face (z = -0.5) - normal = (0, 0, -1)
    vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertex.Tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Right face (x = 0.5) - normal = (1, 0, 0)
    vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertex.Tangent = glm::vec3(0.0f, 0.0f, 1.0f);
    vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Left face (x = -0.5) - normal = (-1, 0, 0)
    vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertex.Tangent = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Top face (y = 0.5) - normal = (0, 1, 0)
    vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertices.push_back(vertex);
    
    // Bottom face (y = -0.5) - normal = (0, -1, 0)
    vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
    vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);
    
    vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
    vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);
    
    // Set indices for the 6 faces (2 triangles per face = 12 triangles)
    // Front face
    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    indices.push_back(2); indices.push_back(3); indices.push_back(0);
    
    // Back face
    indices.push_back(4); indices.push_back(5); indices.push_back(6);
    indices.push_back(6); indices.push_back(7); indices.push_back(4);
    
    // Right face
    indices.push_back(8); indices.push_back(9); indices.push_back(10);
    indices.push_back(10); indices.push_back(11); indices.push_back(8);
    
    // Left face
    indices.push_back(12); indices.push_back(13); indices.push_back(14);
    indices.push_back(14); indices.push_back(15); indices.push_back(12);
    
    // Top face
    indices.push_back(16); indices.push_back(17); indices.push_back(18);
    indices.push_back(18); indices.push_back(19); indices.push_back(16);
    
    // Bottom face
    indices.push_back(20); indices.push_back(21); indices.push_back(22);
    indices.push_back(22); indices.push_back(23); indices.push_back(20);
    
    // Empty texture vector for now
    std::vector<Texture> textures;
    
    // Create mesh and add to the meshes vector
    meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::createGrid(float size, int subdivisions) {
    meshes.clear();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float halfSize = size / 2.0f;

    Vertex vertex;

    // bottom left
    vertex.Position = glm::vec3(-halfSize, 0.0f, -halfSize);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
    vertices.push_back(vertex);

    // bottom right
    vertex.Position = glm::vec3(halfSize, 0.0f, -halfSize);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    vertices.push_back(vertex);

    // top right
    vertex.Position = glm::vec3(halfSize, 0.0f, halfSize);
    vertex.TexCoords = glm::vec2(1.0f, 1.0f);
    vertices.push_back(vertex);

    // top left
    vertex.Position = glm::vec3(-halfSize, 0.0f, halfSize);
    vertex.TexCoords = glm::vec2(0.0f, 1.0f);
    vertices.push_back(vertex);

    // Set indices (2 triangles)
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    std::vector<Texture> textures;

    meshes.push_back(Mesh(vertices, indices, textures));

}