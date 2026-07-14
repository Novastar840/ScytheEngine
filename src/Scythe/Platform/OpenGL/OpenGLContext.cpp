#include "OpenGLContext.h"

#include <glad/gl.h>
#include "GLFW/glfw3.h"
#include <spdlog/spdlog.h>

namespace Scythe
{
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle) {}

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0) {
            spdlog::critical("Failed to initialize GLAD");
            return;
        }
        spdlog::info("OpenGL version {0}", version);
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }

    void OpenGLContext::Resize(int width, int height)
    {
        glViewport(0, 0, width, height);
    }
}
