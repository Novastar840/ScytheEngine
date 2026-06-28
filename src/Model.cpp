#include "Model.h"
#include <spdlog/spdlog.h>
#include <filesystem>

// stb_image implementation (Include exactly ONCE in your project)
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"
#include <glad/gl.h>

#include "assimp/postprocess.h"

namespace Scythe
{
    // Helper function to load image pixels into an OpenGL Texture ID
    unsigned int TextureFromFile(const char* path, const std::string& directory)
    {
        std::filesystem::path filePath = std::filesystem::path(directory) / path;
        std::string filename = filePath.string();

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        // stbi_load reads the image file from disk into RAM
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format = GL_RGB;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            // Upload pixels from RAM to GPU VRAM
            //TODO: Add sRGB support for internalFormat
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data); // Free RAM copy, it's on the GPU now
        }
        else
        {
            spdlog::error("Texture failed to load at path: {}", filename);
            stbi_image_free(data);
        }

        return textureID;
    }

    Model::Model(const std::string& path, const std::string& name, glm::vec3 position)
        : GameObject(name, position)
    {
        loadModel(path);
    }

    void Model::Draw(const Shader& shader) const
    {
        for (const auto& mesh : m_Meshes)
        {
            mesh.Draw(shader);
        }
    }

    void Model::loadModel(const std::string& path)
    {
        Assimp::Importer importer;
        // Read the file with Assimp post-processing steps
        const aiScene* scene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            spdlog::error("ASSIMP ERROR:: {}", importer.GetErrorString());
            return;
        }

        // Store the directory path for later texture loading
        m_Directory = std::filesystem::path(path).parent_path().string();
        processNode(scene->mRootNode, scene);
    }

    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.push_back(processMesh(mesh, scene));
        }
        // Recursively process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
            spdlog::info("Loaded {} meshes", m_Meshes.size());
        }
    }

    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // 1. Extract Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            if (mesh->mNormals)
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }

        // 2. Extract Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // 3. Extract Textures from Materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // Check if texture was already loaded to avoid duplication in memory
            bool skip = false;
            for (unsigned int j = 0; j < m_LoadedTextures.size(); j++)
            {
                if (std::strcmp(m_LoadedTextures[j].Path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(m_LoadedTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                texture.ID = TextureFromFile(str.C_Str(), m_Directory);
                texture.Type = typeName;
                texture.Path = str.C_Str();
                textures.push_back(texture);
                m_LoadedTextures.push_back(texture);
            }
        }
        return textures;
    }
}
