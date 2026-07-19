#include "Shader.h"
#include "RendererAPI.h"
#include "../Platform/OpenGL/OpenGLShader.h"
#include <stdexcept>

namespace Scythe {
    std::shared_ptr<Shader> Shader::Create(const std::string& vertexPath, const std::string& fragmentPath) 
    {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLShader>(vertexPath, fragmentPath);
            case RendererAPI::API::Vulkan:
                throw std::runtime_error("Vulkan Backend is not yet implemented!");
        }
        throw std::runtime_error("Unknown RendererAPI!");
    }
}