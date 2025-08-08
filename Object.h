#ifndef OBJECTS_H
#define OBJECTS_H

#include <math.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Object {
public:
    VAO VAO1;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float angle = 0.0f;
    glm::vec3 pos;
    glm::vec3 col;
    float size;

    void resize(float n);
    virtual void draw(Shader shader, Camera cam, glm::vec3 lightPos, glm::vec3 lightCol, float ar);
};

class Sphere : public Object {
public:
    VBO sphereVBO;
    EBO sphereEBO;
    bool initialized = false;

    Sphere();
    void initialize();

private:
    void generateSphereData(float radius, int sectors, int stacks,
        std::vector<float>& vertices, std::vector<unsigned int>& indices);
};

class Cube : public Object {
public:
    VBO cubeVBO;
    EBO cubeEBO;
    bool initialized = false;

    Cube();
    void initialize();
};

class LightSrc : public Sphere {
public:
    LightSrc();
    void draw(Shader shader, Camera cam, glm::vec3 lightPos, glm::vec3 lightCol, float ar) override;
};


void addShape(std::vector<Object> &objs, int s);

#endif // OBJECTS_H