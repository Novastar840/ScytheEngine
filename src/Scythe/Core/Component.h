#pragma once
#include <cstdint>

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
    };
}
