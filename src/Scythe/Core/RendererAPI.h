#pragma once
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

namespace Scythe
{
    class VertexArray; // Forward declaration for later

    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL = 1,
            Vulkan = 2
        };

        virtual ~RendererAPI() = default;

        // Core lifecycle and state commands
        virtual void Init() = 0;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        // Draw commands (VertexArray will be built in the next step)
        virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

        // The Factory Method
        static std::unique_ptr<RendererAPI> Create();

        // Global API State Accessors
        static API GetAPI() { return s_API; }
        static void SetAPI(API api) { s_API = api; }

    private:
        // Defaults to OpenGL for now
        static API s_API;
    };
