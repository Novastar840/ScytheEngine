#pragma once
#include <memory>
#include <string>
#include <vector>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


namespace Scythe
{
    class Shader;
    class VertexArray;
    class OpenGLShader;
    
    
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };
    
    struct Texture
    {
        unsigned int ID;
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
        // ~Mesh();
        
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        
        void Draw(const std::shared_ptr<Shader>& shader) const;
    };
}
