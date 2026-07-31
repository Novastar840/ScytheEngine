#pragma once
#include "Core/Component.h"
#include <glm/glm.hpp>

namespace Scythe
{
    class TransformComponent;

    enum class ProjectionType { Perspective, Orthographic };

    class CameraComponent : public ComponentImpl<CameraComponent>
    {
    public:
        CameraComponent() = default;

        void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
        void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        void SetAspectRatio(float aspectRatio);

        glm::mat4 GetViewMatrix(const TransformComponent& transform) const;
        glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }

    private:
        void RecalculateProjection();

        ProjectionType m_ProjectionType = ProjectionType::Perspective;
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

        float m_Fov = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.0f;
    };
}
