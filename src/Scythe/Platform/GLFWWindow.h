#pragma once
#include <memory>
#include <string>

#include "../Core/Window.h"

struct GLFWwindow;
namespace Scythe
{
    class GraphicsContext;

    class GLFWWindow : public Window
    {
    public:
        GLFWWindow(const std::string& title, unsigned int width, unsigned int height);
        virtual ~GLFWWindow();

        bool ShouldClose() const override;
        void PollEvents() override;
        void SwapBuffers() override;
        void* GetNativeWindow() const override { return m_Window; }
        double GetTime() const override;

    private:
        GLFWwindow* m_Window;
        std::unique_ptr<GraphicsContext> m_Context;

        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    };
}
