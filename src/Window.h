#pragma once
#include <memory>
#include <string>

struct GLFWwindow;

namespace Scythe
{
    class GraphicsContext;
    class Camera;
    
    class Window
    {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();
        
        bool ShouldClose() const;
        void PollEvents();
        void SwapBuffers();
        
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        GLFWwindow* GetWindow() const { return m_Window; }
        
        void OnResize(int width, int height);
        
        void SetMainCamera(Camera* camera);
        Camera* GetMainCamera() const { return m_MainCamera; }
    private:
        GLFWwindow* m_Window;
        int m_Width;
        int m_Height;
        std::string m_Title;
        Camera* m_MainCamera;
        
        std::unique_ptr<GraphicsContext> m_Context;
        
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    };
}
