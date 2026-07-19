#pragma once
#include "../../Core/Buffer.h"

namespace Scythe
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(uint32_t size);
        ~OpenGLVertexBuffer() override;
        
        void Bind() const override;
        void Unbind() const override;
        void SetData(const void* data, uint32_t size) override;
        
        void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
        const BufferLayout& GetLayout() const override { return m_Layout; }

    private:
        uint32_t m_RendererID;
        BufferLayout m_Layout;
    };
    
    class OpenGLIndexBuffer : public IndexBuffer 
    {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        ~OpenGLIndexBuffer() override;

        void Bind() const override;
        void Unbind() const override;
        
        uint32_t GetCount() const override { return m_Count; }

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };
}