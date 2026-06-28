#pragma once
#include <string>

#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Scythe
{
    class GameObject
    {
    public:
        GameObject(const std::string& name, glm::vec3 position = glm::vec3());
        GameObject(const std::string& name, glm::vec3 position, glm::vec3 rotation);
        GameObject(const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
        
        virtual ~GameObject() = default;
        
        glm::vec3 GetPosition() const {return m_Position;}
        glm::vec3 GetRotation() const {return glm::degrees(glm::eulerAngles(m_Rotation));}
        glm::vec3 GetScale() const {return m_Scale;}
        glm::mat4 GetTransformMatrix() const {return m_TransformMatrix;}
        
        void SetPosition(glm::vec3 newPosition);
        void SetRotation(glm::vec3 newRotation);
        void SetScale(float newScale);
        void SetScale(glm::vec3 newScale);
        
        glm::vec3 GetForwardVector() const;
        glm::vec3 GetRightVector() const;
        glm::vec3 GetUpVector() const;
        
        void LookAtRotation(glm::vec3 lookAtPosition);
        
        virtual void Tick(float deltaTime) {}
    protected:
        void UpdateTransformMatrix();
    private:
        glm::vec3 m_Position;
        glm::quat m_Rotation;
        glm::vec3 m_Scale;
        glm::mat4 m_TransformMatrix;
        std::string m_Name;
    };
}
