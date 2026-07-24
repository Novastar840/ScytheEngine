#pragma once
#include "GameObject.h"
#include "glm/glm.hpp"
#include "glm/detail/type_quat.hpp"

namespace Scythe
{
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    class Camera : public GameObject
    {
    public:
        Camera(const std::string& name, glm::vec3 position = glm::vec3(0.0f));
        ~Camera() override = default;
        
        void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
        void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        
        void SetAspectRatio(float aspectRatio);
        
        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
        glm::mat4 GetViewProjectionMatrix() const;

        virtual void Tick(float deltaTime) override;

    private:
        void RecalculateProjection();

        glm::mat4 m_ProjectionMatrix;
        
        float m_Fov = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.0f;
        ProjectionType m_ProjectionType = ProjectionType::Perspective;
    };
}
