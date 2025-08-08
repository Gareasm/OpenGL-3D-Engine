#include "Object.h"

// Object class implementation
void Object::resize(float n) {
    size = n;
}

void Object::draw(Shader shader, Camera cam, glm::vec3 lightPos, glm::vec3 lightCol, float ar) {
    VAO1.Bind();

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(cam.fov), ar, 0.1f, 100.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(size, size, size));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    shader.Activate();

    shader.setVec3("objectColor", col);
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", cam.pos);
    shader.setVec3("lightColor", lightCol);

    int projLoc1 = glGetUniformLocation(shader.ID, "projection");
    glUniformMatrix4fv(projLoc1, 1, GL_FALSE, glm::value_ptr(projection));

    int viewLoc1 = glGetUniformLocation(shader.ID, "view");
    glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(cam.getViewMatrix()));

    int modelLoc1 = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// Sphere class implementation
Sphere::Sphere() {
    generateSphereData(1.0f, 36, 18, vertices, indices);
    pos = glm::vec3(1.0f, 1.0f, 1.0f);
    col = glm::vec3(1.0f, 0.0f, 0.0f);
    size = 0.3f;

    initialize();
}

void Sphere::initialize() {
    if (initialized) return;

    VAO1.Bind();
    sphereVBO = VBO(vertices.data(), vertices.size() * sizeof(float));
    sphereEBO = EBO(indices.data(), indices.size() * sizeof(unsigned int));

    VAO1.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    initialized = true;
}

void Sphere::generateSphereData(float radius, int sectors, int stacks,
    std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stacks; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;      // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectors+1) vertices per stack
        // the first and last vertices have same position and normal
        for (int j = 0; j <= sectors; ++j) {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < stacks; ++i) {
        k1 = i * (sectors + 1);     // beginning of current stack
        k2 = k1 + sectors + 1;      // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

// Cube class implementation
Cube::Cube() {
    pos = glm::vec3(1.0f, 1.0f, 1.0f);
    col = glm::vec3(1.0f, 0.0f, 0.0f);
    size = 1.0f;

    initialize();
}

void Cube::initialize() {
    if (initialized) return;

    float vertices_arr[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int indices_arr[] = {
        // Each face uses 2 triangles (6 vertices total per face)
        0, 1, 2,    3, 4, 5,      // Back face
        6, 7, 8,    9, 10, 11,    // Front face  
        12, 13, 14, 15, 16, 17,   // Left face
        18, 19, 20, 21, 22, 23,   // Right face
        24, 25, 26, 27, 28, 29,   // Bottom face
        30, 31, 32, 33, 34, 35    // Top face
    };

    size_t sizeV = sizeof(vertices_arr) / sizeof(vertices_arr[0]);
    vertices.assign(vertices_arr, vertices_arr + sizeV);

    size_t sizeI = sizeof(indices_arr) / sizeof(indices_arr[0]);
    indices.assign(indices_arr, indices_arr + sizeI);

    VAO1.Bind();
    cubeVBO = VBO(vertices.data(), vertices.size() * sizeof(float));
    cubeEBO = EBO(indices.data(), indices.size() * sizeof(unsigned int));

    VAO1.LinkAttrib(cubeVBO, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(cubeVBO, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    initialized = true;
}

// LightSrc class implementation
LightSrc::LightSrc() : Sphere() {
    // Call the Sphere constructor
}

void LightSrc::draw(Shader shader, Camera cam, glm::vec3 lightPos, glm::vec3 lightCol, float ar) {
    VAO1.Bind();

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(cam.fov), ar, 0.1f, 100.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(size, size, size));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    shader.Activate();

    shader.setVec3("lightColor", lightCol);

    int projLoc1 = glGetUniformLocation(shader.ID, "projection");
    glUniformMatrix4fv(projLoc1, 1, GL_FALSE, glm::value_ptr(projection));

    int viewLoc1 = glGetUniformLocation(shader.ID, "view");
    glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(cam.getViewMatrix()));

    int modelLoc1 = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

//Emplaces an object 0=CUBE 1=SPHERE
void addShape(std::vector<Object>& objs, int s){
    switch (s) {
        case 0:
            objs.emplace_back(Cube());
            break;
        case 1:
            objs.emplace_back(Sphere());
            break;
     
        default:
            printf("No valid shape declaration");

     }
}