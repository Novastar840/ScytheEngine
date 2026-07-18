#include "OpenGLShader.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace Scythe
{
    Scythe::OpenGLShader::OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSource = ReadFile(vertexPath);
        std::string fragmentSource = ReadFile(fragmentPath);
        
        Compile(vertexSource, fragmentSource);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }
    
    void OpenGLShader::SetInt(const std::string& name, int value) const
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void OpenGLShader::SetFloat(const std::string& name, float value) const
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void OpenGLShader::SetVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    void OpenGLShader::SetVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    void OpenGLShader::SetMat4(const std::string& name, const float* value) const
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
    }

    int OpenGLShader::GetUniformLocation(const std::string& name) const
    {
        auto map = m_UniformLocationCache.find(name);
        if (map != m_UniformLocationCache.end())
        {
            return map->second;
        }

        int location = glGetUniformLocation(m_RendererID, name.c_str());
    
        if (location == -1)
        {
            spdlog::warn("Uniform '{0}' does not exist or is not used in the shader!", name);
        }

        m_UniformLocationCache[name] = location;
        return location;
    }

    std::string OpenGLShader::ReadFile(const std::string& filepath)
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
    
    void OpenGLShader::Compile(const std::string& vertexSource, const std::string& fragmentSource) 
    {
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) return;

        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (fragmentShader == 0) 
        {
            glDeleteShader(vertexShader);
            return;
        }

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);

        GLint isLinked = 0;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE) 
        {
            GLint maxLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);
            glDeleteProgram(m_RendererID);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            spdlog::error("Shader Program Linking Failed:\n{0}", infoLog.data());
            return;
        }

        glDetachShader(m_RendererID, vertexShader);
        glDetachShader(m_RendererID, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    GLuint OpenGLShader::CompileShader(GLenum type, const std::string& source)
    {
        GLuint shader = glCreateShader(type);
        const GLchar* srcCStr = source.c_str();
        glShaderSource(shader, 1, &srcCStr, nullptr);
        glCompileShader(shader);

        GLint isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) 
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
            glDeleteShader(shader);
            spdlog::error("{0} Shader Compilation Failed:\n{1}", 
                          GetShaderTypeName(shader), 
                          infoLog.data());
            return 0;
        }

        return shader;
    }
    
    std::string OpenGLShader::GetShaderTypeName(GLenum type)
    {
        switch (type)
        {
        case GL_VERTEX_SHADER:       return "Vertex";
        case GL_FRAGMENT_SHADER:     return "Fragment";
        case GL_GEOMETRY_SHADER:     return "Geometry";
        default:                     return "Unknown";
        }
    }
}
