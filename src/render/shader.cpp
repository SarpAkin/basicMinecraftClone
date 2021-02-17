#include "shader.h"

#include <iostream>
#include <string>
#include <memory>

#include "opengl_.h"


uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

Shader::Shader(std::string& vertexShader, std::string& fragmentShader)
{
    shaderID = CreateShader(vertexShader, fragmentShader);
}

Shader::Shader(std::string&& vertexShader, std::string&& fragmentShader)
{
    shaderID = CreateShader(vertexShader, fragmentShader);
}

Shader::~Shader()
{
    if (shaderID)
        GLCALL(glDeleteProgram(shaderID));
}

void Shader::Bind() const
{
    GLCALL(glUseProgram(shaderID));
}

void Shader::SetUniform4f(std::string& uniform, float v0, float v1, float v2, float v3)
{
    GLCALL(glUniform4f(getUniformLocation(uniform), v0, v1, v2, v3));
}

void Shader::SetUniform1f(std::string& uniform, float v0)
{
    GLCALL(glUniform1f(getUniformLocation(uniform), v0));
}

void Shader::SetUniform1i(std::string& uniform, int v0)
{
    GLCALL(glUniform1i(getUniformLocation(uniform), v0));
}

void Shader::SetUniformMat4(std::string& uniform, glm::mat4 v0)
{
    GLCALL(glUniformMatrix4fv(getUniformLocation(uniform), 1, GL_FALSE, &v0[0][0]));
}

int Shader::getUniformLocation(std::string& uniformName)
{
    auto it = uniformLocation.find(uniformName);
    if (it != uniformLocation.end())
    {
        return it->second;
    }
    else
    {
        GLCALL(int location = glGetUniformLocation(shaderID, uniformName.c_str()));
        if (location != -1)
        {
            uniformLocation[uniformName] = location;
        }
        else
        {
            std::cerr << "uniform: " << uniformName << " couldn't found exiting!\n";
            exit(-1);
        }
        return location;
    }
}

//shader compilation
uint32_t CompileShader(const std::string& sourceCode, uint32_t ShaderType)
{
    uint32_t id = glCreateShader(ShaderType);
    const char* src = sourceCode.c_str();
    GLCALL(glShaderSource(id, 1, &src, nullptr));
    GLCALL(glCompileShader(id));

    int result;
    GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int lenght;
        GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &lenght));
        auto message = std::make_unique<char>(lenght);
        GLCALL(glGetShaderInfoLog(id, lenght, &lenght, message.get()));
        std::cout << "failed to compile!" << std::endl;
        std::cout << message.get() << std::endl;
        exit(EXIT_FAILURE);
    }

    return id;
}

uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    uint32_t program = glCreateProgram();
    auto vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    auto fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

    GLCALL(glAttachShader(program, vs));
    GLCALL(glAttachShader(program, fs));
    GLCALL(glLinkProgram(program));

    GLCALL(glValidateProgram(program));

    GLCALL(glDeleteShader(vs));
    GLCALL(glDeleteShader(fs));

    return program;
}