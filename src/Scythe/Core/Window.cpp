#include "Window.h"
#include "../Platform/GLFWWindow.h"
#include "../../Camera.h"

namespace Scythe
{
    std::unique_ptr<Window> Window::Create(const std::string& title, unsigned int width, unsigned int height)
    {
        return std::unique_ptr<GLFWWindow>(new GLFWWindow(title, width, height));
    }

    void Window::UpdateOnResize(unsigned int width, unsigned int height)
    {
        m_Width = width;
        m_Height = height;
        
        if (m_MainCamera && height != 0)
        {
            float newAspectRatio = static_cast<float>(width) / static_cast<float>(height);
            m_MainCamera->SetAspectRatio(newAspectRatio);
        }
    }
}
