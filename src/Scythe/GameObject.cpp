#include "GameObject.h"

#include "glm/detail/type_quat.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "spdlog/spdlog.h"

namespace Scythe
{
    GameObject::GameObject(const std::string& name, const glm::vec3 position)
        : m_Position(position), m_Rotation(1.f, 0.f, 0.f, 0.f), m_Scale(1.f), m_Name(name)
    {
        UpdateTransformMatrix();
    }

    GameObject::GameObject(const std::string& name, glm::vec3 position, glm::vec3 rotation)
        : m_Position(position), m_Rotation(glm::quat(glm::radians(rotation))), m_Scale(1.f), m_Name(name)
    {
        UpdateTransformMatrix();
    }

    GameObject::GameObject(const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
        : m_Position(position), m_Rotation(glm::quat(glm::radians(rotation))), m_Scale(scale), m_Name(name)
    {
        UpdateTransformMatrix();
    }

    void GameObject::SetPosition(glm::vec3 newPosition)
    {
        m_Position = newPosition;
        UpdateTransformMatrix();
    }

    void GameObject::SetRotation(glm::vec3 newRotation)
    {
        m_Rotation = glm::quat(glm::radians(newRotation));
        UpdateTransformMatrix();
    }

    void GameObject::SetScale(float newScale)
    {
        m_Scale = glm::vec3(newScale);
        UpdateTransformMatrix();
    }

    void GameObject::SetScale(glm::vec3 newScale)
    {
        m_Scale = newScale;
        UpdateTransformMatrix();
    }

    glm::vec3 GameObject::GetForwardVector() const
    {
        return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 GameObject::GetRightVector() const
    {
        return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 GameObject::GetUpVector() const
    {
        return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    void GameObject::LookAtRotation(glm::vec3 lookAtPosition)
    {
        glm::vec3 direction = lookAtPosition - m_Position;
        
        if (glm::length(direction) < 0.0001f)
        {
            spdlog::error("GameObject{{0}}: look at rotation failed: lookAtPosition to close to position", m_Name);
        }
        
        direction = glm::normalize(direction);
        
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        
        m_Rotation = glm::quatLookAt(direction, worldUp);

        UpdateTransformMatrix();
    }

    void GameObject::UpdateTransformMatrix()
    {
        m_TransformMatrix = glm::mat4(1.0f);
        
        m_TransformMatrix = glm::translate(m_TransformMatrix, m_Position);
        
        // glm::mat4_cast converts a quaternion directly into a rotation matrix.
        m_TransformMatrix *= glm::mat4_cast(m_Rotation);
        
        m_TransformMatrix = glm::scale(m_TransformMatrix, m_Scale);
    }
}
