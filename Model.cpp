// Add this implementation to your model.h or create a model.cpp file
#include "model.h"

void Model::loadModel(string const& path)
{
    // read file via ASSIMP and keep the importer alive
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }

    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // Debug: Print embedded texture info
    cout << "Scene has " << scene->mNumTextures << " embedded textures" << endl;
    for (unsigned int i = 0; i < scene->mNumTextures; i++) {
        aiTexture* tex = scene->mTextures[i];
        cout << "Embedded texture " << i << ": ";
        if (tex->mHeight == 0) {
            cout << "Compressed format, size: " << tex->mWidth << " bytes" << endl;
        }
        else {
            cout << "Uncompressed, " << tex->mWidth << "x" << tex->mHeight << " pixels" << endl;
        }
    }

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            // tangent
            if (mesh->mTangents) {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            }

            // bitangent
            if (mesh->mBitangents) {
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

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

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