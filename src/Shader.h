#pragma once
#include <string>

namespace Scythe
{
    class Shader
    {
    public:
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();
        
        void Bind() const;
        void Unbind() const;
        
        void SetInt(const std::string& name, int value) const;
        void SetFloat(const std::string& name, float value) const;
        void SetMat4(const std::string& name, const float* value) const;
    private:
        unsigned int m_RendererID;
        
        std::string ReadFile(const std::string& filepath);
        unsigned int CompileShader(const std::string& source, unsigned int type);
    };
}
