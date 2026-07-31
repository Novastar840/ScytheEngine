#pragma once
#include "Core/Component.h"
#include <memory>

namespace Scythe
{
    class Model;
    class Shader;

    class MeshRendererComponent : public ComponentImpl<MeshRendererComponent>
    {
    public:
        MeshRendererComponent() = default;

        explicit MeshRendererComponent(std::shared_ptr<Model> model);

        explicit MeshRendererComponent(Model&& model);

        void Draw(const std::shared_ptr<Shader>& shader) const;

    private:
        void OnAttach(GameObject* owner) override;
        std::shared_ptr<Model> m_Model;
    };
}
