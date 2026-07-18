#include "Buffer.h"

#include <stdexcept>

#include "RendererAPI.h"
#include "../Platform/OpenGL/OpenGLBuffer.h"

namespace Scythe
{
    std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size);
            case RendererAPI::API::Vulkan: 
                throw std::runtime_error("Vulkan VertexBuffer not yet implemented!");
        }
        return nullptr;
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
    {
        switch (RendererAPI::GetAPI())
        {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, count);
        case RendererAPI::API::Vulkan: 
                throw std::runtime_error("Vulkan IndexBuffer not yet implemented!");
        }
        return nullptr;
    }
}
