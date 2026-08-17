#pragma once

#include "Core/Component.h"
#include "Components/TransformComponent.h"

namespace Scythe
{
    enum class ProjectionType { Perspective, Orthographic };
    
    struct CameraPerspectiveProperties
    {
        float Fov = 45.0f;
        float AspectRatio = 16.0f / 9.0f;
        float NearPlane = 0.1f;
        float FarPlane = 1000.0f;
    };
    
    struct CameraOrthographicProperties
    {
        float Bottom = -1.0f;
        float Top = 1.0f;
        float Left = -16.f / 9;
        float Right = 16.f / 9;
        float NearPlane = 0.1f;
        float FarPlane = 1000.0f;
    };
    
    class CameraComponent : public ComponentImpl<CameraComponent>
    {
        SCYTHE_COMPONENT(CameraComponent)
        DECLARE_COMPONENT_DEPENDENCY(TransformComponent);
        
    public:
        CameraComponent() = default;
        CameraComponent(const CameraPerspectiveProperties& properties);

        void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
        void SetPerspective(const CameraPerspectiveProperties& properties);
        void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        void SetOrthographic(const CameraOrthographicProperties& properties);
        void SetAspectRatio(float aspectRatio);

        Mat4 GetViewMatrix(const TransformComponent& transform) const;
        Mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }

    private:
        void RecalculateProjection();

        ProjectionType m_ProjectionType = ProjectionType::Perspective;
        Mat4 m_ProjectionMatrix = Mat4(1.0f);

        float m_Fov = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.0f;
    };
}
