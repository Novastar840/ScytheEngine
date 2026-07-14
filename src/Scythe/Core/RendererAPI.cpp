#include "RendererAPI.h"

#include <stdexcept>
#include "../Platform/OpenGL/OpenGLRendererAPI.h"

namespace Scythe
{
    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
    std::unique_ptr<RendererAPI> RendererAPI::s_Instance;
    
    std::unique_ptr<RendererAPI> RendererAPI::Create() 
    {
        switch (s_API) 
        {
        case API::OpenGL:   
            return std::make_unique<OpenGLRendererAPI>();
            
        case API::Vulkan:   
            // We will implement this in Phase 4. 
            // Throwing an error prevents silent nullptr crashes later.
            throw std::runtime_error("Vulkan API is selected but not yet implemented!");
            
        case API::None:
        default: 
            throw std::runtime_error("Unknown or unsupported RendererAPI!");
        }
    }
}
