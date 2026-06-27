#include "Window.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

namespace Scythe
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        spdlog::error("GLFW Error ({0}): {1}", error, description);
    }

    Window::Window(int width, int height, const std::string& title)
        : m_Width(width), m_Height(height), m_Title(title)
    {
        spdlog::info("Creating window '{0}' ({1}x{2})", title, width, height);
        
        glfwSetErrorCallback(GLFWErrorCallback);
        
        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW!");
            return;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            spdlog::critical("Failed to create GLFW window");
            glfwTerminate();
            return;
        }
        
        glfwMakeContextCurrent(m_Window);
        
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0)
        {
            spdlog::error("Failed to initialize GLAD");
            return;
        }
        
        glEnable(GL_DEPTH_TEST);
        
        spdlog::info("OpenGL version {0}", version);
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
        glfwSwapBuffers(m_Window);
    }

    void Window::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
