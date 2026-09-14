#include "SceneObject.h"

namespace Scythe
{
    SceneObject::SceneObject(const SceneObject& other)
    : GameObject(other)
    {
        m_TransformComponent = other.m_TransformComponent;
    }
    
    SceneObject& SceneObject::operator=(const SceneObject& other)
    {
        if (this == &other)
            return *this;

        GameObject::operator=(other);
        m_TransformComponent = other.m_TransformComponent;
        return *this;
    }

    SceneObject& SceneObject::operator=(SceneObject&& other) noexcept
    {
        if (this == &other)
            return *this;

        GameObject::operator=(std::move(other));
        m_TransformComponent = std::move(other.m_TransformComponent);
        return *this;
    }

    bool SceneObject::HasComponentTypeID(uint32_t typeID) const
    {
        if (typeID == TransformComponent::StaticTypeID()) return true;
        return GameObject::HasComponentTypeID(typeID);
    }

    Component* SceneObject::GetComponentByID(uint32_t typeID) const
    {
        if (typeID == TransformComponent::StaticTypeID()) 
            return const_cast<TransformComponent*>(&m_TransformComponent);
            
        return GameObject::GetComponentByID(typeID);
    }

    SceneObject::SceneObject(SceneObject&& other) noexcept
        : GameObject(std::move(other))
    {
        m_TransformComponent = std::move(other.m_TransformComponent);
    }
}
