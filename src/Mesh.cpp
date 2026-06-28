#include "Mesh.h"

#include "shader.h"
#include <glad/gl.h>

#include "spdlog/spdlog.h"

namespace Scythe
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : m_Vertices(std::move(vertices)), m_Indices(std::move(indices)), m_Textures(std::move(textures))
    {
        SetupMesh();
        spdlog::info("Mesh setup completed");
    }

    Mesh::~Mesh()
    {
        if (m_VAO != 0)
        {
            spdlog::info("Deleting buffers of mesh");
        }
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    Mesh::Mesh(Mesh&& other) noexcept 
        : m_Vertices(std::move(other.m_Vertices)),
          m_Indices(std::move(other.m_Indices)),
          m_Textures(std::move(other.m_Textures)),
          m_VAO(other.m_VAO),
          m_VBO(other.m_VBO),
          m_EBO(other.m_EBO)
    {
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            // Free existing resources
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);

            // Transfer ownership
            m_Vertices = std::move(other.m_Vertices);
            m_Indices = std::move(other.m_Indices);
            m_Textures = std::move(other.m_Textures);
            
            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_EBO = other.m_EBO;

            // Invalidate source
            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_EBO = 0;
        }
        return *this;
    }
    
    void Mesh::Draw(const Shader& shader) const
    {
        if(!m_VAO)
        {
            spdlog::warn("Attempting to draw mesh without a VAO");
        }
        
        shader.SetInt("hasTexture", m_Textures.empty() ? 0 : 1);

        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;

        for(unsigned int i = 0; i < m_Textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
            
            std::string number;
            std::string name = m_Textures[i].Type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);

            shader.SetInt((name + number).c_str(), i);

            glBindTexture(GL_TEXTURE_2D, m_Textures[i].ID);
        }

        // Draw mesh
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // Reset active texture to 0 for good practice
        glActiveTexture(GL_TEXTURE0);
    }

    void Mesh::SetupMesh()
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        
        glBindVertexArray(m_VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(m_Vertices.size() * sizeof(Vertex)), &m_Vertices[0], GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(m_Indices.size() * sizeof(unsigned int)), &m_Indices[0], GL_STATIC_DRAW);
        
        // Vertex Positions (Location 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // Vertex Normals (Location 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        
        // Vertex Texture Coords (Location 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
}
