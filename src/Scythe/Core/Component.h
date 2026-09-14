#pragma once
#include <cstdint>

#include "ComponentDependencyMeta.h"

#define DECLARE_COMPONENT_DEPENDENCY(...) using Requires = TypeList<__VA_ARGS__>

namespace Scythe
{
    class Component
    {
        friend class GameObject;
    public:
        virtual ~Component() = default;
        
        virtual uint32_t GetTypeID() const = 0;
        
        virtual void Update(float deltaTime) {}
        virtual std::unique_ptr<Component> Clone() const = 0;
        
        GameObject* GetOwner() const { return m_Owner; }
        virtual bool RequiresComponentType(uint32_t typeID) const { return false; }
    protected:
        Component() = default;
        GameObject* m_Owner = nullptr;
        
        virtual void OnAttach(GameObject* owner) {}
        virtual void OnDetach() {}
        
        static inline uint32_t s_NextTypeID = 0;
    };
    
    template <typename T>
    class ComponentImpl : public Component
    {
    public:
        static uint32_t StaticTypeID()
        {
            static uint32_t id = s_NextTypeID++;
            return id;
        }
        
        uint32_t GetTypeID() const override
        {
            return StaticTypeID();
        }
        
        std::unique_ptr<Component> Clone() const override
        {
            static_assert(std::is_copy_constructible_v<T>,
                          "Components must be copy-constructible to support Scene deep-copy");
            return std::make_unique<T>(static_cast<const T&>(*this));
        }
        
        bool RequiresComponentType(uint32_t typeID) const override
        {
            return RequiresImpl(typeID, ComponentRequiresList<T>{});
        }
        
        template <typename Func>
        static void ForEachRequiredTypeID(Func&& func)
        {
            ForEachRequiredTypeIDImpl(std::forward<Func>(func), ComponentRequiresList<T>{});
        }
        
    private:
        template <typename... Deps>
        static bool RequiresImpl(uint32_t typeID, TypeList<Deps...>)
        {
            return ((Deps::StaticTypeID() == typeID) || ...);
        }
        
        template <typename Func, typename... Deps>
        static void ForEachRequiredTypeIDImpl(Func&& func, TypeList<Deps...>)
        {
            (func(Deps::StaticTypeID()), ...);
        }
    };
    
    template <typename T, typename... Args>
        requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
    std::unique_ptr<T> MakeComponent(Args&&... args) 
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}
