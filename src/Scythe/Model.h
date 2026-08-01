#pragma once
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "GameObject.h"

namespace Scythe
{
    class Shader;

    class Model
    {
    public:
        Model(const std::string& path, const std::string& name);
        Model(const std::string&& path, const std::string&& name);
        void Draw(const std::shared_ptr<Shader>& shader) const;

    private:
        std::vector<Mesh> m_Meshes;
        std::string m_Directory;
        std::string m_Name;
        std::vector<Texture> m_LoadedTextures;
        
        void loadModel(const std::string& path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    };
}
