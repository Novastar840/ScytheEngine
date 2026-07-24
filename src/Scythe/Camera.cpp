#include "Camera.h"

namespace Scythe
{
    Camera::Camera(const std::string& name, glm::vec3 position)
        : GameObject(name, position)
    {
        SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    }

    void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
    {
        m_Fov = fovDegrees;
        m_AspectRatio = aspectRatio;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
        m_ProjectionType = ProjectionType::Perspective;
        RecalculateProjection();
    }

    void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }

    void Camera::SetAspectRatio(float aspectRatio)
    {
        m_AspectRatio = aspectRatio;
        RecalculateProjection();
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        glm::vec3 target = GetPosition() + GetForwardVector();
        
        return glm::lookAt(GetPosition(), target, GetUpVector());
    }

    glm::mat4 Camera::GetViewProjectionMatrix() const
    {
        return m_ProjectionMatrix * GetViewMatrix();
    }

    void Camera::Tick(float deltaTime)
    {
        GameObject::Tick(deltaTime);
    }

    void Camera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective) 
        {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
        }
    }
}
