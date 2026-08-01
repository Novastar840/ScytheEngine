#include "TransformComponent.h"
#include "glm/ext/matrix_transform.hpp"
#include "spdlog/spdlog.h"

namespace Scythe
{
    TransformComponent::TransformComponent(Vec3 position, Quat rotation, Vec3 scale)
        : m_Position(position), m_Rotation(rotation), m_Scale(scale)
    {
    }

    void TransformComponent::SetPosition(Vec3 newPosition)
    {
        if (m_Position != newPosition)
        {
            m_Position = newPosition;
            m_MatrixDirty = true;
        }
    }

    void TransformComponent::SetRotation(Vec3 newRotation)
    {
        SetRotation(Quat(glm::radians(newRotation)));
    }

    void TransformComponent::SetRotation(Quat newRotation)
    {
        if (m_Rotation != newRotation)
        {
            m_Rotation = newRotation;
            m_MatrixDirty = true;
        }
    }

    Vec3 TransformComponent::GetEulerAngles() const
    {
        return glm::degrees(glm::eulerAngles(m_Rotation));
    }

    void TransformComponent::SetScale(float newScale)
    {
        SetScale(Vec3(newScale));
    }

    void TransformComponent::SetScale(Vec3 newScale)
    {
        if (m_Scale != newScale)
        {
            m_Scale = newScale;
            m_MatrixDirty = true;
        }
    }

    Mat4 TransformComponent::GetTransformMatrix() const
    {
        if (m_MatrixDirty) UpdateTransformMatrix();
        return m_TransformMatrix;
    }

    Vec3 TransformComponent::GetForwardVector() const
    {
        return m_Rotation * Vec3(0.0f, 0.0f, -1.0f);
    }

    Vec3 TransformComponent::GetRightVector() const
    {
        return m_Rotation * Vec3(1.0f, 0.0f, 0.0f);
    }

    Vec3 TransformComponent::GetUpVector() const
    {
        return m_Rotation * Vec3(0.0f, 1.0f, 0.0f);
    }

    void TransformComponent::LookAtRotation(Vec3 lookAtPosition)
    {
        Vec3 direction = lookAtPosition - m_Position;
        if (glm::length(direction) < 0.0001f)
        {
            spdlog::error("TransformComponent: look at rotation failed: lookAtPosition too close");
            return;
        }
        direction = glm::normalize(direction);
        auto worldUp = Vec3(0.0f, 1.0f, 0.0f);
        SetRotation(glm::quatLookAt(direction, worldUp));
    }

    void TransformComponent::UpdateTransformMatrix() const
    {
        m_TransformMatrix = Mat4(1.0f);
        m_TransformMatrix = glm::translate(m_TransformMatrix, m_Position);
        m_TransformMatrix *= glm::mat4_cast(m_Rotation);
        m_TransformMatrix = glm::scale(m_TransformMatrix, m_Scale);
        m_MatrixDirty = false;
    }
}
