#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include <glm/mat4x4.hpp>

class Shader
{
private:
    uint32_t shaderID = 0;
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

    // Uniform related
    void SetUniform4f(std::string uniform, float v0, float v1, float v2, float v3);
    void SetUniform3f(std::string uniform, glm::vec3);
    void SetUniform1f(std::string uniform, float v0);
    void SetUniform1i(std::string uniform, int v0);
    void SetUniformMat4(std::string uniform, glm::mat4 v0);

    void SetUniform3f(std::string uniform, glm::vec3*,int count);

    inline Shader(Shader&& other)
    {
        shaderID = other.shaderID;
        other.shaderID = 0;
    }
    inline void operator=(Shader&& other)
    {
        this->~Shader();
        uniformLocation = std::move(other.uniformLocation);
        shaderID = other.shaderID;
        other.shaderID = 0;
    }
};
