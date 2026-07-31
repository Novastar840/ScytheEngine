#include "TransformComponent.h"
#include "glm/ext/matrix_transform.hpp"
#include "spdlog/spdlog.h"

namespace Scythe
{
    TransformComponent::TransformComponent(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
        : m_Position(position), m_Rotation(rotation), m_Scale(scale)
    {
    }

    void TransformComponent::SetPosition(glm::vec3 newPosition)
    {
        if (m_Position != newPosition)
        {
            m_Position = newPosition;
            m_MatrixDirty = true;
        }
    }

    void TransformComponent::SetRotation(glm::vec3 newRotation)
    {
        SetRotation(glm::quat(glm::radians(newRotation)));
    }

    void TransformComponent::SetRotation(glm::quat newRotation)
    {
        if (m_Rotation != newRotation)
        {
            m_Rotation = newRotation;
            m_MatrixDirty = true;
        }
    }

    glm::vec3 TransformComponent::GetEulerAngles() const
    {
        return glm::degrees(glm::eulerAngles(m_Rotation));
    }

    void TransformComponent::SetScale(float newScale)
    {
        SetScale(glm::vec3(newScale));
    }

    void TransformComponent::SetScale(glm::vec3 newScale)
    {
        if (m_Scale != newScale)
        {
            m_Scale = newScale;
            m_MatrixDirty = true;
        }
    }

    glm::mat4 TransformComponent::GetTransformMatrix() const
    {
        if (m_MatrixDirty) UpdateTransformMatrix();
        return m_TransformMatrix;
    }

    glm::vec3 TransformComponent::GetForwardVector() const
    {
        return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 TransformComponent::GetRightVector() const
    {
        return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 TransformComponent::GetUpVector() const
    {
        return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    void TransformComponent::LookAtRotation(glm::vec3 lookAtPosition)
    {
        glm::vec3 direction = lookAtPosition - m_Position;
        if (glm::length(direction) < 0.0001f)
        {
            spdlog::error("TransformComponent: look at rotation failed: lookAtPosition too close");
            return;
        }
        direction = glm::normalize(direction);
        auto worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        SetRotation(glm::quatLookAt(direction, worldUp));
    }

    void TransformComponent::UpdateTransformMatrix() const
    {
        m_TransformMatrix = glm::mat4(1.0f);
        m_TransformMatrix = glm::translate(m_TransformMatrix, m_Position);
        m_TransformMatrix *= glm::mat4_cast(m_Rotation);
        m_TransformMatrix = glm::scale(m_TransformMatrix, m_Scale);
        m_MatrixDirty = false;
    }
}
