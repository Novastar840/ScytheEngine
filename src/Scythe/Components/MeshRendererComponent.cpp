#include "MeshRendererComponent.h"
#include "TransformComponent.h"
#include "Model.h"
#include "Core/Shader.h"
#include <spdlog/spdlog.h>
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
        if (!owner->HasComponent<TransformComponent>())
        {
            spdlog::error("MeshRendererComponent on '{}' requires a TransformComponent!", owner->GetName());
        }
    }

    void MeshRendererComponent::Draw(const std::shared_ptr<Shader>& shader) const
    {
        if (!m_Model) return;

        const auto* transform = m_Owner->GetComponent<TransformComponent>();
        if (transform)
        {
            shader->SetMat4("uModel", glm::value_ptr(transform->GetTransformMatrix()));
        }

        m_Model->Draw(shader);
    }
}
