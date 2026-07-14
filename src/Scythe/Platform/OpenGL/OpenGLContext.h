#pragma once
#include "../../Core/GraphicsContext.h"

struct GLFWwindow;

namespace Scythe
{
    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* windowHandle);
        ~OpenGLContext() override = default;
        
        void Init() override;
        void SwapBuffers() override;
        void Resize(int width, int height) override;
    private:
        GLFWwindow* m_WindowHandle;
    };
}
