#pragma once
#include "GameObject.h"
#include "glm/vec3.hpp"

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
        
        template <typename... Ts>
        GameObject* CreateGameObject(const std::string& name, std::unique_ptr<Ts>... components)
        {
            m_GameObjects.emplace_back(std::make_unique<GameObject>(name, std::move(components)...));
            return m_GameObjects.back().get();
        }
        
        void DestroyGameObject(const GameObject* target);
        int GetGameObjectCount() const { return static_cast<int>(m_GameObjects.size()); }
        
        void Update(float deltaTime);
    private:
        std::string m_Name;
        std::vector<std::unique_ptr<GameObject>> m_GameObjects;
    };
}
