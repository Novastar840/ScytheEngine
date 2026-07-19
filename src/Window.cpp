#include "Window.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include "Camera.h"
#include <spdlog/spdlog.h>

#include "Scythe/Core/GraphicsContext.h"
#include "Scythe/Core/RendererAPI.h"

namespace Scythe
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        spdlog::error("GLFW Error ({0}): {1}", error, description);
    }

    Window::Window(int width, int height, const std::string& title)
        : m_Width(width), m_Height(height), m_Title(title), m_MainCamera(nullptr)
    {
        spdlog::info("Creating window '{0}' ({1}x{2})", title, width, height);
        
        glfwSetErrorCallback(GLFWErrorCallback);
        
        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW!");
            return;
        }
        
        if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); 
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        } else if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        
        m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            spdlog::critical("Failed to create GLFW window");
            glfwTerminate();
            return;
        }
        
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
        
        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Init();
    }

    Window::~Window()
    {
        spdlog::info("Destroying window...");
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    bool Window::ShouldClose() const
    {
        return  glfwWindowShouldClose(m_Window);
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::SwapBuffers()
    {
        m_Context->SwapBuffers();
    }

    void Window::OnResize(int width, int height)
    {
        m_Width = width;
        m_Height = height;
        m_Context->Resize(width, height);
        
        if (m_MainCamera && height != 0)
        {
            float newAspectRatio = static_cast<float>(width) / static_cast<float>(height);
            m_MainCamera->SetAspectRatio(newAspectRatio);
        }
    }

    double Window::GetTime() const
    {
        return glfwGetTime();
    }

    void Window::SetMainCamera(Camera* camera)
    {
        m_MainCamera = camera;
    }

    void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win)
        {
            win->OnResize(width, height);
        }
    }
}
