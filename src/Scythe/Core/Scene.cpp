#include "Scene.h"

#include "Components/TransformComponent.h"

namespace Scythe
{
    Scene::Scene(const std::string& name)
        : m_Name(name)
    {
    }

    Scene::Scene(std::string&& name)
        : m_Name(std::move(name))
    {
    }

    GameObject* Scene::CreateGameObject(
        const std::string& name, 
        const Vec3& position, 
        const Vec3& rotation, 
        const Vec3& scale)
    {
        m_GameObjects.emplace_back(std::make_unique<GameObject>(
            name, 
            MakeComponent<TransformComponent>(position, rotation, scale)));
        return m_GameObjects.back().get();
    }

    GameObject* Scene::CreateGameObject(const std::string& name, const Vec3& position)
    {
        m_GameObjects.emplace_back(std::make_unique<GameObject>(
            name, 
            MakeComponent<TransformComponent>(position)));
        return m_GameObjects.back().get();
    }

    void Scene::DestroyGameObject(const GameObject* target)
    {
        for (auto& gameObject : m_GameObjects)
        {
            if (gameObject.get() == target)
            {
                std::swap(gameObject, m_GameObjects.back());
                m_GameObjects.pop_back();
                return;
            }
        }
    }

    void Scene::Update(float deltaTime)
    {
        for (auto& gameObject : m_GameObjects)
        {
            gameObject->Update(deltaTime);
        }
    }
}
