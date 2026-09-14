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

        GameObject& operator=(const GameObject& other);
        GameObject& operator=(GameObject&& other) noexcept;
        
        template <typename... StaticComps, typename... Ts>
            requires (std::derived_from<Ts, Component> && ...)
        GameObject(StaticComponentsTag<StaticComps...>, const std::string& name, UniquePtr<Ts>... components)
            : m_ID(++s_NextID), m_Name(name)
        {
            using Available = TypeList<StaticComps..., Ts...>;
            static_assert(PackDependenciesSatisfied<Available>::value,
                "GameObject constructor pack is missing required components");
            
            if constexpr (sizeof...(Ts) > 0)
            {
                m_Components.reserve(sizeof...(Ts));
                (AttachRaw(std::move(components)), ...);    
            }
        }
        
        template <typename... Ts>
            requires (std::derived_from<Ts, Component> && ...)
        GameObject(const std::string& name, UniquePtr<Ts>... components)
            : m_ID(++s_NextID), m_Name(name) 
        {
            using Available = TypeList<Ts...>;
            static_assert(PackDependenciesSatisfied<Available>::value,
                "GameObject constructor pack is missing required components");
            
            if constexpr (sizeof...(Ts) > 0)
            {
                m_Components.reserve(sizeof...(Ts));
                (AttachRaw(std::move(components)), ...);    
            }
        }
        
        virtual bool HasComponentTypeID(uint32_t typeID) const;
        virtual Component* GetComponentByID(uint32_t typeID) const;

        virtual ~GameObject() = default;

        ID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        template <typename T, typename... Args>
            requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
        T* AddComponent(Args&&... args)
        {
            static_assert(!TypeListContains<T, ComponentRequiresList<T>>::value,
                "A component cannot list itself as a dependency");
            static_assert(AllDeriveFromComponent<ComponentRequiresList<T>>::value,
                "All required types must derive from Component");
            static_assert(!HasCircularDependency<T>::value,
                "Circular dependency detected in component dependency graph");

            bool deps_satisfied = true;
            T::ForEachRequiredTypeID([&](uint32_t reqID) 
            {
                if (!this->HasComponentTypeID(reqID)) 
                {
                    deps_satisfied = false;
                }
            });

            if (!deps_satisfied) 
            {
                spdlog::error("Cannot add component '{}': missing required component", typeid(T).name());
                return nullptr;
            }
            
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
            return static_cast<T*>(GetComponentByID(T::StaticTypeID()));
        }

        template <typename T>
            requires std::derived_from<T, ComponentImpl<T>>
        bool HasComponent() const
        {
            return HasComponentTypeID(T::StaticTypeID());
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
        void DetachAllComponents();
        
    private:
        static inline uint64_t s_NextID = 0;
    };
}
    