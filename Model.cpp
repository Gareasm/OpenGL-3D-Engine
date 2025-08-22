// Add this implementation to your model.h or create a model.cpp file
#include "model.h"
void Model::loadModel(string const& path)
{
    // Use better flags for GLB files - especially important for larger models
    unsigned int flags = aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        //aiProcess_FlipUVs |          // Important for GLB files
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FixInfacingNormals |
        aiProcess_OptimizeMeshes;

    scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    // Debug scene information
    cout << "=== SCENE DEBUG INFO ===" << endl;
    cout << "Root node children: " << scene->mRootNode->mNumChildren << endl;
    cout << "Total meshes: " << scene->mNumMeshes << endl;
    cout << "Materials: " << scene->mNumMaterials << endl;
    cout << "Embedded textures: " << scene->mNumTextures << endl;

    // Print root node transformation
    aiMatrix4x4& rootTransform = scene->mRootNode->mTransformation;
    cout << "Root transform: [" << rootTransform.a1 << "," << rootTransform.a2 << "," << rootTransform.a3 << "," << rootTransform.a4 << "]" << endl;

    // Process with identity matrix initially
    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

void Model::processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform)
{
    // Convert assimp matrix to glm matrix
    glm::mat4 nodeTransform = aiMatrix4x4ToGlm(node->mTransformation);

    // Apply parent transformation
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    // Debug node information
    cout << "Processing node: " << node->mName.C_Str()
        << " with " << node->mNumMeshes << " meshes" << endl;

    // Process each mesh in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, globalTransform));
    }

    // Process child nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, globalTransform);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 transform)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // Debug mesh information
    cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << endl;

    // Process vertices with transformation applied
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Apply transformation to vertex position
        glm::vec4 pos = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        pos = transform * pos;
        vertex.Position = glm::vec3(pos.x, pos.y, pos.z);

        // Transform normals (use inverse transpose for correct normal transformation)
        if (mesh->HasNormals())
        {
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));
            glm::vec3 normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vertex.Normal = glm::normalize(normalMatrix * normal);
        }
        else
        {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default normal
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            // Transform tangents if available
            if (mesh->mTangents) {
                glm::vec3 tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                vertex.Tangent = glm::normalize(glm::mat3(transform) * tangent);
            }

            if (mesh->mBitangents) {
                glm::vec3 bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
                vertex.Bitangent = glm::normalize(glm::mat3(transform) * bitangent);
            }
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        // Initialize bone data to zero
        for (int j = 0; j < MAX_BONE_INFLUENCE; j++) {
            vertex.m_BoneIDs[j] = 0;
            vertex.m_Weights[j] = 0.0f;
        }

        vertices.push_back(vertex);
    }

    // Process indices (unchanged)
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Process materials (unchanged)
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Texture texture;
            // Pass 'this' to allow access to embedded textures
            texture.id = TextureFromFile(str.C_Str(), this->directory, false, this);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int Model::loadEmbeddedTexture(const char* path)
{
    if (!scene || path[0] != '*') {
        return 0;
    }

    // Extract texture index from path (remove '*' prefix)
    string pathStr(path);
    int textureIndex;
    try {
        textureIndex = std::stoi(pathStr.substr(1));
    }
    catch (const std::exception& e) {
        cout << "Invalid embedded texture index: " << path << endl;
        return 0;
    }

    if (textureIndex >= 0 && textureIndex < static_cast<int>(scene->mNumTextures)) {
        aiTexture* texture = scene->mTextures[textureIndex];

        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        if (texture->mHeight == 0) {
            // Compressed texture format (JPEG, PNG, etc.)
            cout << "Loading compressed embedded texture " << textureIndex
                << " (size: " << texture->mWidth << " bytes)" << endl;

            int width, height, nrComponents;
            unsigned char* data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(texture->pcData),
                texture->mWidth,
                &width, &height, &nrComponents, 0
            );

            if (data) {
                GLenum format;
                if (nrComponents == 1) format = GL_RED;
                else if (nrComponents == 3) format = GL_RGB;
                else if (nrComponents == 4) format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                cout << "Successfully loaded embedded texture " << textureIndex
                    << " (" << width << "x" << height << ", " << nrComponents << " components)" << endl;

                stbi_image_free(data);
            }
            else {
                cout << "Failed to decode embedded texture " << textureIndex << endl;
                // Create a default colored texture as fallback
                unsigned char defaultData[] = { 128, 128, 255, 255 }; // Light blue
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultData);
            }
        }
        else {
            // Uncompressed texture data
            cout << "Loading uncompressed embedded texture " << textureIndex
                << " (" << texture->mWidth << "x" << texture->mHeight << ")" << endl;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->mWidth, texture->mHeight,
                0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pcData);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }
    else {
        cout << "Invalid embedded texture index: " << textureIndex
            << " (max: " << scene->mNumTextures - 1 << ")" << endl;
    }

    return 0;
}

// Updated TextureFromFile function
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma, Model* model)
{
    string filename = string(path);

    // Handle embedded textures first
    if (filename[0] == '*' && model) {
        unsigned int embeddedTexture = model->loadEmbeddedTexture(path);
        if (embeddedTexture != 0) {
            return embeddedTexture;
        }

        // If embedded texture loading failed, create a default texture
        cout << "Embedded texture loading failed for " << path << ", creating default" << endl;
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Create a distinctive color based on the texture index
        int index = 0;
        try {
            index = std::stoi(filename.substr(1));
        }
        catch (...) {}

        unsigned char data[4];
        switch (index % 8) {
        case 0: data[0] = 255; data[1] = 100; data[2] = 100; break; // Red
        case 1: data[0] = 100; data[1] = 255; data[2] = 100; break; // Green
        case 2: data[0] = 100; data[1] = 100; data[2] = 255; break; // Blue
        case 3: data[0] = 255; data[1] = 255; data[2] = 100; break; // Yellow
        case 4: data[0] = 255; data[1] = 100; data[2] = 255; break; // Magenta
        case 5: data[0] = 100; data[1] = 255; data[2] = 255; break; // Cyan
        case 6: data[0] = 255; data[1] = 150; data[2] = 100; break; // Orange
        case 7: data[0] = 150; data[1] = 100; data[2] = 255; break; // Purple
        }
        data[3] = 255;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

    // Handle regular file textures
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        cout << "Loaded file texture: " << filename << endl;
    }
    else
    {
        cout << "Texture failed to load at path: " << filename << endl;
        stbi_image_free(data);

        // Create default white texture
        glBindTexture(GL_TEXTURE_2D, textureID);
        unsigned char defaultData[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return textureID;
}