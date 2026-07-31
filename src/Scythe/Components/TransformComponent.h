#pragma once
#include <string>

#include "Core/Component.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Scythe
{
    class TransformComponent : public ComponentImpl<TransformComponent>
    {
    public:
        TransformComponent(
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3 scale = glm::vec3(1.0f));

        glm::vec3 GetPosition() const { return m_Position; }
        void SetPosition(glm::vec3 newPosition);

        glm::quat GetRotation() const { return m_Rotation; }
        glm::vec3 GetEulerAngles() const;
        void SetRotation(glm::vec3 newRotation);
        void SetRotation(glm::quat newRotation);

        glm::vec3 GetScale() const { return m_Scale; }
        void SetScale(float newScale);
        void SetScale(glm::vec3 newScale);

        glm::mat4 GetTransformMatrix() const;

        glm::vec3 GetForwardVector() const;
        glm::vec3 GetRightVector() const;
        glm::vec3 GetUpVector() const;
        void LookAtRotation(glm::vec3 lookAtPosition);

    private:
        void UpdateTransformMatrix() const;

        glm::vec3 m_Position;
        glm::quat m_Rotation;
        glm::vec3 m_Scale;

        mutable glm::mat4 m_TransformMatrix;
        mutable bool m_MatrixDirty = true;
    };
}
