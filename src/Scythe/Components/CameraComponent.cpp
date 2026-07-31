#include "CameraComponent.h"
#include "TransformComponent.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Scythe
{
    void CameraComponent::SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_Fov = fovDegrees;
        m_AspectRatio = aspectRatio;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
        RecalculateProjection();
    }

    void CameraComponent::SetOrthographic(float left, float right, float bottom, float top, float nearPlane,
                                          float farPlane)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }

    void CameraComponent::SetAspectRatio(float aspectRatio)
    {
        m_AspectRatio = aspectRatio;
        RecalculateProjection();
    }

    glm::mat4 CameraComponent::GetViewMatrix(const TransformComponent& transform) const
    {
        glm::vec3 position = transform.GetPosition();
        glm::vec3 target = position + transform.GetForwardVector();
        return glm::lookAt(position, target, transform.GetUpVector());
    }

    void CameraComponent::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
        }
    }
}
