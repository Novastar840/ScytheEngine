#pragma once

#include <unordered_map>
#include <glad/gl.h>

#include "../../Core/Shader.h"

namespace Scythe
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath);
        ~OpenGLShader() override;
        
        void Bind() const override;
        void Unbind() const override;

        void SetInt(const std::string& name, int value) const override;
        void SetFloat(const std::string& name, float value) const override;
        void SetVec3(const std::string& name, const glm::vec3& value) const override;
        void SetVec4(const std::string& name, const glm::vec4& value) const override;
        void SetMat4(const std::string& name, const float* value) const override;
    private:
        uint32_t m_RendererID;
        
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;
        
        int GetUniformLocation(const std::string& name) const;
        std::string ReadFile(const std::string& filepath);
        void Compile(const std::string& vertexSource, const std::string& fragmentSource);
        static GLuint CompileShader(GLenum type, const std::string& source);
        static std::string GetShaderTypeName(GLenum type);
    };
}
