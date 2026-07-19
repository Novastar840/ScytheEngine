#include "Texture2D.h"

#include <stdexcept>

#include "RendererAPI.h"
#include "../Platform/OpenGL/OpenGLTexture2D.h"

namespace Scythe
{
    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLTexture2D>(path);
            case RendererAPI::API::Vulkan:
                throw std::runtime_error("Vulkan Backend is not yet implemented!");
        }
        throw std::runtime_error("Unknown RendererAPI!");
    }
}
