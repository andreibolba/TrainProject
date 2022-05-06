#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 

//#include <GL/glew.h>
#include <GL/glew.h>
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//#include <glad/glad.h>
#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

#ifndef SHADER_H
#define SHADER_H

class Shader
{
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    void use();
    void setBool(const std::string& name, bool value) const;    
    void setInt(const std::string& name, int value) const;    
    void setFloat(const std::string& name, float value) const; 
    void setVec2(const std::string& name, const glm::vec2& value) const;   
    void setVec2(const std::string& name, float x, float y) const;  
    void setVec3(const std::string& name, const glm::vec3& value) const;  
    void setVec3(const std::string& name, float x, float y, float z) const; 
    void setVec4(const std::string& name, const glm::vec4& value) const;   
    void setVec4(const std::string& name, float x, float y, float z, float w);    
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
  
private:
    void checkCompileErrors(GLuint shader, std::string type);   
};
#endif