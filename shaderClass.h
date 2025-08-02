#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include <glm/glm.hpp>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

class Shader {
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);
	void Activate();
	void Delete();

        void setBool(const std::string& name, bool value);

        // ------------------------------------------------------------------------
        void setInt(const std::string& name, int value);

        // ------------------------------------------------------------------------
        void setFloat(const std::string& name, float value);

        // ------------------------------------------------------------------------
        void setVec2(const std::string& name, const glm::vec2& value);

        void setVec2(const std::string& name, float x, float y);

        // ------------------------------------------------------------------------
        void setVec3(const std::string& name, const glm::vec3& value);

        void setVec3(const std::string& name, float x, float y, float z);

        // ------------------------------------------------------------------------
        void setVec4(const std::string& name, const glm::vec4& value);

        void setVec4(const std::string& name, float x, float y, float z, float w);

        // ------------------------------------------------------------------------
        void setMat2(const std::string& name, const glm::mat2& mat);

        // ------------------------------------------------------------------------
        void setMat3(const std::string& name, const glm::mat3& mat);

        // ------------------------------------------------------------------------
        void setMat4(const std::string& name, const glm::mat4& mat);
    
private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
};


#endif