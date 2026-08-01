#pragma once
#include <cstdint>
#include <memory>

namespace Scythe
{
    class GameObject;

    class Component
    {
        friend class GameObject;
    public:
        virtual ~Component() = default;
        
        virtual uint32_t GetTypeID() const = 0;
        
        virtual void OnAttach(GameObject* owner) {}
        virtual void OnDetach() {}
        virtual void Update(float deltaTime) {}
        virtual std::unique_ptr<Component> Clone() const = 0;
        
        GameObject* GetOwner() const { return m_Owner; }
    protected:
        Component() = default;
        GameObject* m_Owner;
        
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
    };
    
    template <typename T, typename... Args>
        requires std::derived_from<T, ComponentImpl<T>> && std::constructible_from<T, Args...>
    std::unique_ptr<T> MakeComponent(Args&&... args) 
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}
