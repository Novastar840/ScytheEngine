#include "GLFWWindow.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "Core/GraphicsContext.h"
#include "Core/RendererAPI.h"

namespace Scythe
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        spdlog::error("GLFW Error ({0}): {1}", error, description);
    }

    static bool s_GLFWInitialized = false;

    GLFWWindow::GLFWWindow(const std::string& title, unsigned int width, unsigned int height)
        : Window(title, width, height)
    {
        spdlog::info("Creating window '{0}' ({1}x{2})", title, width, height);

        if (!s_GLFWInitialized)
        {
            glfwSetErrorCallback(GLFWErrorCallback);
            if (!glfwInit())
            {
                spdlog::critical("Failed to initialize GLFW!");
                return;
            }
            s_GLFWInitialized = true;
        }

        if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        else if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), m_Title.c_str(), nullptr,
                                    nullptr);
        if (!m_Window)
        {
            spdlog::critical("Failed to create GLFW window");
            glfwTerminate();
            return;
        }

        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

        m_Context = std::unique_ptr(GraphicsContext::Create(m_Window));
        m_Context->Init();
    }

    GLFWWindow::~GLFWWindow()
    {
        spdlog::info("Destroying window...");
        if (m_Window)
        {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
        
        if (s_GLFWInitialized)
        {
            glfwTerminate();
            s_GLFWInitialized = false;
        }
    }

    bool GLFWWindow::ShouldClose() const { return glfwWindowShouldClose(m_Window); }
    void GLFWWindow::PollEvents() { glfwPollEvents(); }
    void GLFWWindow::SwapBuffers() { m_Context->SwapBuffers(); }
    double GLFWWindow::GetTime() const { return glfwGetTime(); }

    void GLFWWindow::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto win = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (win)
        {
            win->UpdateOnResize(width, height);
            win->m_Context->Resize(width, height);
        }
    }
}
