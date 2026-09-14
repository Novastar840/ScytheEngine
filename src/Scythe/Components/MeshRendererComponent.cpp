#include "MeshRendererComponent.h"
#include "TransformComponent.h"
#include "Model.h"
#include "Core/Shader.h"
#include <glm/gtc/type_ptr.hpp>

namespace Scythe
{
    MeshRendererComponent::MeshRendererComponent(std::shared_ptr<Model> model)
        : m_Model(model)
    {
    }

    MeshRendererComponent::MeshRendererComponent(Model&& model)
        : m_Model(std::make_shared<Model>(std::move(model)))
    {
    }

    void MeshRendererComponent::OnAttach(GameObject* owner)
    {
        m_TransformComponent = owner->GetComponent<TransformComponent>();
        if (!m_TransformComponent)
        {
            spdlog::error("MeshRendererComponent on '{}' requires a TransformComponent!", owner->GetName());
        }
    }

    void MeshRendererComponent::Draw(const std::shared_ptr<Shader>& shader) const
    {
        if (!m_Model) return;

        if (m_TransformComponent)
        {
            shader->SetMat4("uModel", glm::value_ptr(m_TransformComponent->GetTransformMatrix()));
        }

        m_Model->Draw(shader);
    }
}
