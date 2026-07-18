#pragma once
#include <memory>
#include <string>

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Scythe
{
    class Shader
    {
    public:
        virtual ~Shader() = default;
        
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        
        virtual void SetInt(const std::string& name, int value) const = 0;
        virtual void SetFloat(const std::string& name, float value) const = 0;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) const = 0;
        virtual void SetVec4(const std::string& name, const glm::vec4& value) const = 0;
        virtual void SetMat4(const std::string& name, const float* value) const = 0;
        
        static std::shared_ptr<Shader> Create(const std::string& name, const std::string& fragmentPath);
    };
}
