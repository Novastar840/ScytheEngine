#pragma once
#include <cstdint>

namespace Scythe
{
    enum class ShaderDataType
    {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };
    
    constexpr uint32_t GetShaderDataTypeSize(ShaderDataType type)
    {
        constexpr std::array<uint32_t, 12> sizes = {
            0,   // None
            4,   // Float
            8,   // Float2
            12,  // Float3
            16,  // Float4
            36,  // Mat3
            64,  // Mat4
            4,   // Int
            8,   // Int2
            12,  // Int3
            16,  // Int4
            1    // Bool
        };
        return sizes[static_cast<size_t>(type)];
    }

    constexpr uint32_t GetShaderDataTypeComponentCount(ShaderDataType type)
    {
        constexpr std::array<uint32_t, 12> counts = {
            0,   // None
            1,   // Float
            2,   // Float2
            3,   // Float3
            4,   // Float4
            9,   // Mat3
            16,  // Mat4
            1,   // Int
            2,   // Int2
            3,   // Int3
            4,   // Int4
            1    // Bool
        };
        return counts[static_cast<size_t>(type)];
    }
    
    struct BufferElement
    {
        std::string Name;
        ShaderDataType Type;
        uint32_t Size;
        size_t Offset;
        bool Normalized;

        BufferElement() = default;

        BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
            : Name(name), Type(type), Size(GetShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
        {
        }
    };

    class BufferLayout
    {
    public:
        BufferLayout() = default;

        BufferLayout(std::initializer_list<BufferElement> elements) 
            : m_Elements(elements)
        {
            CalculateOffsetsAndStride();
        }

        uint32_t GetStride() const { return m_Stride; }
        const std::vector<BufferElement>& GetElements() const { return m_Elements; }

    private:
        void CalculateOffsetsAndStride()
        {
            size_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }

        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride = 0;
    };

    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual void SetData(const void* data, uint32_t size) = 0;
        virtual void SetLayout(const BufferLayout& layout) = 0;
        virtual const BufferLayout& GetLayout() const = 0;

        static std::shared_ptr<VertexBuffer> Create(uint32_t size);
    };

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual uint32_t GetCount() const = 0;

        static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);
    };
}
