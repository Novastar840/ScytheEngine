#pragma once
#include <memory>

namespace Scythe
{
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;
        virtual void Init() = 0;
        virtual void SwapBuffers() = 0;
        virtual void Resize(int width, int height) = 0;

        static std::unique_ptr<GraphicsContext> Create(void* window);
    };
}
