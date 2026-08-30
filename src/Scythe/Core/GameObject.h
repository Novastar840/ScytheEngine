#pragma once

#include <utility>

#include "Component.h"
#include "Components/TransformComponent.h"

namespace Scythe
{
    class GameObject
    {
        SCYTHE_GAME_OBJECT(GameObject)
    public:
        using ID = uint64_t;
        static constexpr ID INVALID_ID = 0xFFFFFFFFFFFFFFFFULL;

        GameObject(const std::string& name = "");
        
        GameObject(const GameObject& other);
        GameObject(GameObject&& other) noexcept;
        
        template <typename... Ts>
            requires (sizeof...(Ts) > 0 && (std::derived_from<Ts, Component> && ...))
        GameObject(const std::string& name, std::unique_ptr<Ts>... components)
            : m_ID(++s_NextID), m_Name(name)
        {
            static_assert(Component::PackDependenciesSatisfied<Component::TypeList<Ts...>>::value,
                "GameObject constructor pack is missing required components");
            
            m_Components.reserve(sizeof...(Ts));
            (AttachRaw(std::move(components)), ...); 
        }

        virtual ~GameObject() = default;

        ID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        template <typename T, typename... Args>
            requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
        T* AddComponent(Args&&... args)
        {
            static_assert(!Component::TypeListContains<T, Component::ComponentRequiresList<T>>::value,
                "A component cannot list itself as a dependency");
            static_assert(Component::AllDeriveFromComponent<Component::ComponentRequiresList<T>>::value,
                "All required types must derive from Component");
            static_assert(!Component::HasCircularDependency<T>::value,
                "Circular dependency detected in component dependency graph");

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            Component* rawPtr = component.get();

            m_Components.push_back(std::move(component));
            rawPtr->m_Owner = this;
            rawPtr->OnAttach(this);

            return static_cast<T*>(rawPtr);
        }

        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        T* GetComponent() const
        {
            for (const auto& comp : m_Components)
            {
                if (comp->GetTypeID() == T::StaticTypeID())
                {
                    return static_cast<T*>(comp.get());
                }
            }
            return nullptr;
        }

        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        bool HasComponent() const
        {
            return GetComponent<T>() != nullptr;
        }

        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        void RemoveComponent()
        {
            auto it= std::find_if(
                m_Components.begin(), 
                m_Components.end(),
                [](const std::unique_ptr<Component>& comp)
                {
                    return comp->GetTypeID() == T::StaticTypeID();
                });
            if (it != m_Components.end())
            {
                (*it)->OnDetach();
                m_Components.erase(it);
            }
        }

        void Update(float deltaTime) const
        {
            for (auto& comp : m_Components)
            {
                comp->Update(deltaTime);
            }
        }
    
    protected:
        ID m_ID = INVALID_ID;
        std::string m_Name;
        std::vector<std::unique_ptr<Component>> m_Components;
        
        void AttachRaw(std::unique_ptr<Component> component);
        
    private:
        static inline uint64_t s_NextID = 0;
    };
    
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
                : GameObject(name, MakeComponent<TransformComponent>(pos, rot, scale), std::move(components)...)
        {
            Component* rawPtr = m_Components.front().get();
            m_TransformComponent = static_cast<TransformComponent*>(rawPtr);
        }
        
        SceneObject(const SceneObject& other);
        SceneObject(SceneObject&& other) noexcept;
        
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
                return GetComponent<T>() != nullptr;
            }
        }
        
        TransformComponent* GetTransformComponent() const {return m_TransformComponent;}
        Vec3 GetPosition() const {return m_TransformComponent->GetPosition();}
        Quat GetRotation() const {return m_TransformComponent->GetRotation();}
        Vec3 GetScale() const {return m_TransformComponent->GetScale();}
    private:
        TransformComponent* m_TransformComponent;
    };
}
    