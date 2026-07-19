#pragma once
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

namespace Scythe
{
    class VertexArray;

    class RendererAPI
    {
    public:
        enum class API
        {
            OpenGL = 1,
            Vulkan = 2
        };

        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

        static std::unique_ptr<RendererAPI> Create();

        static API GetAPI() { return s_API; }
        static void SetAPI(API api) { s_API = api; }
        
        static void Initialize() { s_Instance = Create(); s_Instance->Init(); }
        static RendererAPI* Get() { return s_Instance.get(); }
    private:
        static API s_API;
        static std::unique_ptr<RendererAPI> s_Instance;
    };
}