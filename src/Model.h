#pragma once
#include <string>
#include <vector>
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Scythe
{
    class Model {
    public:
        Model(const std::string& path);
        void Draw(const Shader& shader) const;

    private:
        std::vector<Mesh> m_Meshes;
        std::string m_Directory;
        std::vector<Texture> m_LoadedTextures; // Optimization: Prevents loading the same texture twice
        
        void loadModel(const std::string& path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    };
}