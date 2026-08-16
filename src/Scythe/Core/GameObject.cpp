#include "GameObject.h"

namespace Scythe
{
    GameObject::GameObject(const std::string& name)
        : m_ID(++s_NextID), m_Name(name)
    {
    }
    
    GameObject::GameObject(GameObject&& other) noexcept
        : m_ID(std::exchange(other.m_ID, INVALID_ID)),
        m_Name(std::move(other.m_Name)),
        m_Components(std::move(other.m_Components))
    {
        for (auto& comp : m_Components) 
        {
            comp->m_Owner = this;
        }
    }
    
    GameObject::GameObject(const GameObject& other)
        : m_ID(s_NextID++), m_Name(other.m_Name)
    {
        m_Components.reserve(other.m_Components.size());
        for (const auto& component : other.m_Components)
        {
            auto cloned = component->Clone();
            Component* rawPtr = cloned.get();
            m_Components.push_back(std::move(cloned));
            rawPtr->m_Owner = this;
            rawPtr->OnAttach(this);
        }
    }

    void GameObject::AttachRaw(std::unique_ptr<Component> component)
    {
        Component* rawPtr = component.get();
        m_Components.push_back(std::move(component));
        rawPtr->m_Owner = this;
        rawPtr->OnAttach(this);
    }
}
