#include "Shader.h"
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Shader::Shader(const std::string& filepath)
	: m_Filepath(filepath), m_RendererID(0)
{
    ShaderProgramSource shaderSource = ParseShader(filepath);
    m_RendererID = CreateShader(shaderSource.VertexSource, shaderSource.FragmentSource);

    // Debugging
   /* std::cout << "VERTEX SHADER SOURCE" << std::endl;
    std::cout << shaderSource.FragmentSource << std::endl;
    std::cout << "FRAGMENT SHADER SOURCE" << std::endl;
    std::cout << shaderSource.FragmentSource << std::endl;*/
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    GLCall(unsigned int shaderProgram = glCreateProgram());
    // Compile each shader individually
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    // Link both shaders to shaderProgram
    GLCall(glAttachShader(shaderProgram, vs));
    GLCall(glAttachShader(shaderProgram, fs));
    GLCall(glLinkProgram(shaderProgram));
    GLCall(glValidateProgram(shaderProgram));
    // Delete intermediate values
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));
    return shaderProgram;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* shader_src = source.c_str();
    glShaderSource(id, 1, &shader_src, nullptr);
    glCompileShader(id);

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) {
        // Error handling
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char)); // dynamic stack allocation
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "[ERROR]: " << (type == GL_VERTEX_SHADER ? "Vertex " : "Fragment ") << "shader compile failed." << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
    // Read in a single file which contains both the vertex and fragment shader source codes
    std::ifstream stream(filepath);
    enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };
    ShaderType currentType = ShaderType::NONE;
    std::string currentLine;
    std::stringstream stringStream[2];
    while (getline(stream, currentLine))
    {
        if (currentLine.find("#shader") != std::string::npos)
        {
            // Update the shader type
            if (currentLine.find("vertex") != std::string::npos)
            {
                // Set mode to vertex shader
                currentType = ShaderType::VERTEX;
            }
            else if (currentLine.find("fragment") != std::string::npos)
            {
                // Set mode to fragment shader
                currentType = ShaderType::FRAGMENT;
            }
        }
        else
        {
            // Append current line to proper string stream
            stringStream[(int)currentType] << currentLine << "\n";
        }
    }
    return ShaderProgramSource{ stringStream[0].str(), stringStream[1].str() };
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetVec3f(const std::string& name, float v0, float v1, float v2)
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetVec3(const std::string& name,glm::vec3 vector)
{
    GLCall(glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetMatrix4f(const std::string& name, const glm::mat4& matrix4)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix4[0][0]));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetInt(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetBool(const std::string& name, bool value)
{
    GLCall(glUniform1i(GetUniformLocation(name), (int)value));
}

void Shader::SetFloat(const std::string& name, float value)
{
    int uniformLocation = GetUniformLocation(name);
	GLCall(glUniform1f(uniformLocation, value));
}

int Shader::GetUniformLocation(const std::string& name) const
{
    // Caching of uniform locations means we don't have to call glGetUniformLocation() every time we want to set a uniform
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(int uniformLocation = glGetUniformLocation(m_RendererID, name.c_str()));
    if (uniformLocation == -1)
    {
        std::cout << "[Warning] Uniform named " << name << " does not exist!" << std::endl;
    }
    else 
    {
        // Cacher uniform location only if found
        m_UniformLocationCache[name] = uniformLocation;
    }
    return uniformLocation;
}
