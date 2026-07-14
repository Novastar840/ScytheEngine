#include "GraphicsContext.h"

#include <stdexcept>
#include "../Platform/OpenGL/OpenGLContext.h"
#include "RendererAPI.h"

struct GLFWwindow;

namespace Scythe
{
    std::unique_ptr<GraphicsContext> GraphicsContext::Create(void* window)
    {
        switch (RendererAPI::GetAPI())
        {
        case RendererAPI::API::None:    return nullptr;
        case RendererAPI::API::OpenGL:  return std::make_unique<OpenGLContext>(static_cast<GLFWwindow*>(window));
        case RendererAPI::API::Vulkan:  throw std::runtime_error("Vulkan Context not yet implemented!");
        }
        return nullptr;
    }
}
