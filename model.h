#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shaderClass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false, class Model* model = nullptr);

class Model
{
public:
    glm::vec3 pos;
    glm::vec3 angle;
 
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // Keep scene and importer as members for embedded texture access
    const aiScene* scene;
    Assimp::Importer importer;

    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        angle = glm::vec3(0.0f, 0.0f, 0.0f);

        loadModel(path);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    unsigned int loadEmbeddedTexture(const char* path);

private:
    void loadModel(string const& path);
    void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform = glm::mat4(1.0f));
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 transform);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

    // Helper function to convert aiMatrix4x4 to glm::mat4
    glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }
};

#endif