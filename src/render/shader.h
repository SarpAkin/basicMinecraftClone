#pragma once 

#include <string>
#include <iostream>
#include <unordered_map>

#include <glm/mat4x4.hpp>


class Shader
{
private:
    uint32_t shaderID;
    std::unordered_map<std::string, int> uniformLocation;
private:
    int getUniformLocation(std::string&);
private:
public:
    Shader() = default;
    Shader(std::string& vertexShader, std::string& fragmentShader);
    Shader(std::string&& vertexShader, std::string&& fragmentShader);
    ~Shader();

    void Bind() const;

    //Uniform related
    void SetUniform4f(std::string& uniform, float v0, float v1, float v2, float v3);
    void SetUniform1f(std::string& uniform, float v0);
    void SetUniform1i(std::string& uniform, int v0);
    void SetUniformMat4(std::string& uniform, glm::mat4 v0);

    inline Shader(Shader&& other){shaderID = other.shaderID;other.shaderID = 0;}
    inline void operator =(Shader&& other){shaderID = other.shaderID;other.shaderID = 0;}


    inline void SetUniform4f(const char* uniform, float v0, float v1, float v2, float v3)
    {
        std::string uniform_ = uniform; return SetUniform4f(uniform_, v0, v1, v2, v3);
    }
    inline void SetUniform1f(const char* uniform, float v0)
    {
        std::string uniform_ = uniform; return SetUniform1f(uniform_, v0);
    }
    inline void SetUniform1i(const char* uniform, int v0)
    {
        std::string uniform_ = uniform; return SetUniform1i(uniform_, v0);
    }
    inline void SetUniformMat4(const char* uniform, glm::mat4 v0)
    {
        std::string uniform_ = uniform; return SetUniformMat4(uniform_, v0);
    }
};

