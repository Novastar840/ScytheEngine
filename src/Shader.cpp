#include "Shader.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace Scythe
{
    Scythe::Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSource = ReadFile(vertexPath);
        std::string fragmentSource = ReadFile(fragmentPath);
        
        unsigned int vs = CompileShader(vertexSource, GL_VERTEX_SHADER);
        unsigned int fs = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vs);
        glAttachShader(m_RendererID, fs);
        glLinkProgram(m_RendererID);

        // Check for linking errors
        int success;
        char infoLog[512];
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_RendererID, 512, NULL, infoLog);
            spdlog::error("Shader Program Linking Failed:\n{0}", infoLog);
        }

        // Clean up individual shaders (they are now baked into the program)
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }
    
    void Shader::SetInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
    }

    void Shader::SetFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), value);
    }

    void Shader::SetMat4(const std::string& name, const float* value) const {
        glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), 1, GL_FALSE, value);
    }
    
    std::string Shader::ReadFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            spdlog::error("Could not open shader file: {0}", filepath);
            spdlog::error("Make sure your 'assets' folder is being copied to the build directory!");
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        if (content.empty()) {
            spdlog::error("Shader file is EMPTY: {0}", filepath);
        }
        
        return content;
    }
    
    unsigned int Shader::CompileShader(const std::string& source, unsigned int type) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int success;
        char infoLog[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            std::string typeName = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
            spdlog::error("{0} Shader Compilation Failed:\n{1}", typeName, infoLog);
        }
        return id;
    }
}
