#pragma once
#include <utility>
#include "GameObject.h"
#include "Reflection.h"

namespace Scythe
{
    class SceneObject : public GameObject
    {
        SCYTHE_GAME_OBJECT(SceneObject)
    public:
        template<typename... Ts>
        SceneObject(
            const std::string& name = "",
            Vec3 pos = Vec3(0.f), 
            Quat rot = Quat(1.0f, 0.0f, 0.0f, 0.0f), 
            Vec3 scale = Vec3(1.f),
            std::unique_ptr<Ts>... components)
                : GameObject(name, std::move(components)...), m_TransformComponent(pos, rot, scale)
        {
        }
        
        SceneObject(const SceneObject& other);
        SceneObject(SceneObject&& other) noexcept;

        SceneObject& operator=(const SceneObject& other);
        SceneObject& operator=(SceneObject&& other) noexcept;

        template <typename T, typename... Args>
            requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
        T* AddComponent(Args&&... args)
        {
            if constexpr (std::is_same_v<T, TransformComponent>)
            {
                spdlog::warn("Adding a transform component to the scene object is disallowed");
                return nullptr;
            }
            else
            {
                return GameObject::AddComponent<T>(std::forward<Args>(args)...);
            }
        }
        
        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        T* GetComponent() const
        {
            if constexpr (std::is_same_v<T, TransformComponent>)
            {
                return const_cast<T*>(&m_TransformComponent);
            }
            else
            {
                return GameObject::GetComponent<T>();
            }
        }
        
        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        void RemoveComponent()
        {
            if constexpr (std::is_same_v<T, TransformComponent>)
            {
                spdlog::warn("Removing a transform component to the scene object is disallowed");
            }
            else
            {
                GameObject::RemoveComponent<T>();
            }
        }
        
        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        bool HasComponent() const
        {
            if constexpr (std::is_same_v<T, TransformComponent>)
            {
                return true;
            }
            else
            {
                return GameObject::HasComponent<T>();
            }
        }
        
        TransformComponent* GetTransformComponent() const {return const_cast<TransformComponent*>(&m_TransformComponent);}
        Vec3 GetPosition() const {return m_TransformComponent.GetPosition();}
        Quat GetRotation() const {return m_TransformComponent.GetRotation();}
        Vec3 GetScale() const {return m_TransformComponent.GetScale();}
        
        void LookAtRotation(Vec3 lookAtPosition) {m_TransformComponent.LookAtRotation(lookAtPosition);}
    private:
        TransformComponent m_TransformComponent;
    };   
}
