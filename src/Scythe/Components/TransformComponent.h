#pragma once

#include "Core/Component.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

// Coordinate convention: X-forward, Y-up, Z-right.

namespace Scythe {
    class TransformComponent : public ComponentImpl<TransformComponent>
    {
        SCYTHE_COMPONENT(TransformComponent)
    public:
        TransformComponent(
            Vec3 position = Vec3(0.0f),
            Quat rotation = Quat(1.0f, 0.0f, 0.0f, 0.0f),
            Vec3 scale = Vec3(1.0f));

        Vec3 GetPosition() const { return m_Position; }
        void SetPosition(Vec3 newPosition);

        Quat GetRotation() const { return m_Rotation; }
        Vec3 GetEulerAngles() const;
        void SetRotation(Vec3 newRotation);
        void SetRotation(Quat newRotation);

        Vec3 GetScale() const { return m_Scale; }
        void SetScale(float newScale);
        void SetScale(Vec3 newScale);

        Mat4 GetTransformMatrix() const;

        /// Returns the object's local forward direction (X-axis by convention).
        Vec3 GetForwardVector() const;
        /// Returns the object's local right direction (Z-axis by convention).
        Vec3 GetRightVector() const;
        /// Returns the object's local up direction (Y-axis by convention).
        Vec3 GetUpVector() const;
        void LookAtRotation(Vec3 lookAtPosition);

    private:
        void UpdateTransformMatrix() const;

        Vec3 m_Position;
        Quat m_Rotation;
        Vec3 m_Scale;

        mutable Mat4 m_TransformMatrix;
        mutable bool m_MatrixDirty = true;
    };
}
