#include "TransformComponent.h"
#include "Core/CoordinateSystem.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "spdlog/spdlog.h"

namespace Scythe
{
    /// Build a rotation quaternion that aligns this object's forward axis to 'forwardDir'.
    /// Convention:
    /// local +X = forward
    /// local +Y = up
    /// local +Z = right
    static Quat makeLookAtQuat(Vec3 forwardDir, Vec3 inWorldUp)
    {
        if (glm::length(forwardDir) < 0.0001f)
        {
            return Quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        Vec3 forward = glm::normalize(forwardDir);
        Vec3 referenceUp = glm::normalize(inWorldUp);

        // If looking almost straight up/down, world up is not a valid reference.
        // Pick another reference vector so we can still build a valid orientation.
        if (glm::abs(glm::dot(forward, referenceUp)) > 0.9999f)
        {
            referenceUp = RightVector;
        }
        
        Vec3 right = glm::normalize(glm::cross(forward, referenceUp));
        Vec3 up = glm::cross(right, forward);
        
        Mat4 rotMat(1.0f);
        rotMat[0] = Vec4(forward, 0.0f);
        rotMat[1] = Vec4(up, 0.0f);
        rotMat[2] = Vec4(right, 0.0f);

        return glm::normalize(glm::quat_cast(rotMat));
    }

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
        return m_Rotation * ForwardVector;
    }

    Vec3 TransformComponent::GetRightVector() const
    {
        return m_Rotation * RightVector;
    }

    Vec3 TransformComponent::GetUpVector() const
    {
        return m_Rotation * UpVector;
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
        SetRotation(makeLookAtQuat(direction, UpVector));
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
