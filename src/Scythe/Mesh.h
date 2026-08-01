#pragma once

namespace Scythe
{
    class Shader;
    class VertexArray;
    class Texture2D;
    
    
    struct Vertex
    {
        Vec3 Position;
        Vec3 Normal;
        Vec2 TexCoords;
    };
    
    struct Texture
    {
        std::shared_ptr<Texture2D> Image;
        std::string Type;
        std::string Path;
    };
    
    class Mesh
    {
    public:
        std::vector<Vertex> m_Vertices;
        std::vector<unsigned int> m_Indices;
        std::vector<Texture> m_Textures;
        
        std::shared_ptr<VertexArray> m_VertexArray;
        
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        
        void Draw(const std::shared_ptr<Shader>& shader) const;
    };
}
