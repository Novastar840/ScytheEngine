#pragma once
#include <memory>

#include "Components/CameraComponent.h"

struct GLFWWindow;

namespace Scythe
{
    class GraphicsContext;
    class Camera;
    
    class Window
    {
    public:
        virtual bool ShouldClose() const = 0;
        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;
        virtual void* GetNativeWindow() const = 0;
        virtual double GetTime() const = 0;
        
        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }
        const std::string& GetTitle() const { return m_Title; }
        
        void SetMainCamera(CameraComponent* camera) { m_MainCamera = camera; }
        CameraComponent* GetMainCamera() const { return m_MainCamera; }
        
        static std::unique_ptr<Window> Create(
            const std::string& title = "Scythe Engine", 
            unsigned int width = 800, 
            unsigned int height = 600);
    protected:
        unsigned int m_Width, m_Height;
        std::string m_Title;
        CameraComponent* m_MainCamera = nullptr;
        
        Window(const std::string& title, unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height), m_Title(title) {}
        
        void UpdateOnResize(unsigned int width, unsigned int height);
    };
}
