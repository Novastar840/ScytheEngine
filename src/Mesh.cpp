#include "Mesh.h"

#include "Scythe/Core/RendererAPI.h"
#include "Scythe/Core/Shader.h"
#include "Scythe/Core/Texture2D.h"
#include "Scythe/Core/VertexArray.h"
#include "spdlog/spdlog.h"

namespace Scythe
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : m_Vertices(std::move(vertices)), m_Indices(std::move(indices)), m_Textures(std::move(textures))
    {
        m_VertexArray = VertexArray::Create();
        
        auto vertexBuffer = VertexBuffer::Create(m_Vertices.size() * sizeof(Vertex));
        vertexBuffer->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "aPos" },
            { ShaderDataType::Float3, "aNormal" },
            { ShaderDataType::Float2, "aTexCoords" }
        });
        m_VertexArray->AddVertexBuffer(vertexBuffer);
        
        auto indexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
        m_VertexArray->SetIndexBuffer(indexBuffer);
        
        spdlog::info("Mesh setup completed");
    }

    Mesh::Mesh(Mesh&& other) noexcept 
        : m_Vertices(std::move(other.m_Vertices)),
          m_Indices(std::move(other.m_Indices)),
          m_Textures(std::move(other.m_Textures)),
          m_VertexArray(std::move(other.m_VertexArray))
    {
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            m_Vertices = std::move(other.m_Vertices);
            m_Indices = std::move(other.m_Indices);
            m_Textures = std::move(other.m_Textures);
            m_VertexArray = std::move(other.m_VertexArray);
        }
        return *this;
    }
    
    void Mesh::Draw(const std::shared_ptr<Shader>& shader) const
    {
        shader->SetInt("hasTexture", m_Textures.empty() ? 0 : 1);

        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;

        for(unsigned int i = 0; i < m_Textures.size(); i++) {
            std::string number;
            std::string name = m_Textures[i].Type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);

            shader->SetInt((name + number).c_str(), i);

            if (m_Textures[i].Image) {
                m_Textures[i].Image->Bind(i);
            }
        }

        // Draw mesh
        m_VertexArray->Bind();
        RendererAPI::Get()->DrawIndexed(m_VertexArray); 
    }
}
