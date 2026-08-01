#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "Core/Component.h"

namespace Scythe
{
    class GameObject
    {
    public:
        using ID = uint64_t;
        static constexpr ID INVALID_ID = 0xFFFFFFFFFFFFFFFFULL;

        GameObject(const std::string& name = "");
        
        GameObject(GameObject&& other) noexcept;
        
        GameObject(const GameObject& other);
        
        template <typename... Ts>
            requires (sizeof...(Ts) > 0 && (std::derived_from<Ts, Component> && ...))
        GameObject(const std::string& name, std::unique_ptr<Ts>... components)
            : m_ID(++s_NextID), m_Name(name)
        {
            m_Components.reserve(sizeof...(Ts));
            
            (AttachRaw(std::move(components)), ...); 
        }

        virtual ~GameObject() = default;

        ID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        template <typename T, typename... Args>
            requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of_v<Component, T>,
                          "AddComponent requires a type derived from Scythe::Component");

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            Component* rawPtr = component.get();

            m_Components.push_back(std::move(component));
            rawPtr->m_Owner = this;
            rawPtr->OnAttach(this);

            return static_cast<T&>(*rawPtr);
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

    private:
        ID m_ID = INVALID_ID;
        std::string m_Name;
        std::vector<std::unique_ptr<Component>> m_Components;

        static inline uint64_t s_NextID = 0;
        
        void AttachRaw(std::unique_ptr<Component> component);
    };
}
