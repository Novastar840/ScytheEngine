#pragma once
#include "Model.h"

namespace Scythe
{
    class Scene
    {
    public:
        Scene(const std::string& name);
        Scene(std::string&& name);
        
        GameObject* CreateGameObject(
            const std::string& name, 
            const glm::vec3& position, 
            const glm::vec3& rotation, 
            const glm::vec3& scale = glm::vec3(1.0f)
            );
        
        GameObject* CreateGameObject(const std::string& name = "", const glm::vec3& position = glm::vec3(0.0f));
        
        void DestroyGameObject(const GameObject* target);
        
        void Update(float deltaTime);
    private:
        std::string m_Name;
        std::vector<std::unique_ptr<GameObject>> m_GameObjects;
    };
}
