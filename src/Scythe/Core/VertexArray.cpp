#include "VertexArray.h"

#include <stdexcept>

#include "RendererAPI.h"
#include "../Platform/OpenGL/OpenGLVertexArray.h"

namespace Scythe
{
    std::shared_ptr<VertexArray> VertexArray::Create()
    {
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLVertexArray>();
            case RendererAPI::API::Vulkan:
                throw std::runtime_error("Vulkan Backend is not yet implemented!");
        }
        throw std::runtime_error("Unknown RendererAPI!");
    }
}
