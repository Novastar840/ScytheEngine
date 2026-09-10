#include "SceneObject.h"

namespace Scythe
{
    SceneObject::SceneObject(const SceneObject& other)
    : GameObject(other)
    {
        m_TransformComponent = GetComponent<TransformComponent>();
    }
    
    SceneObject& SceneObject::operator=(const SceneObject& other)
    {
        if (this == &other)
            return *this;

        GameObject::operator=(other);
        m_TransformComponent = GetComponent<TransformComponent>();
        return *this;
    }

    SceneObject& SceneObject::operator=(SceneObject&& other) noexcept
    {
        if (this == &other)
            return *this;

        GameObject::operator=(std::move(other));
        m_TransformComponent = other.m_TransformComponent;
        other.m_TransformComponent = nullptr;
        return *this;
    }

    SceneObject::SceneObject(SceneObject&& other) noexcept
        : GameObject(std::move(other))
    {
        m_TransformComponent = other.m_TransformComponent;
        other.m_TransformComponent = nullptr;
    }
}
