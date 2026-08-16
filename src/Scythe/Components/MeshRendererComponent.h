#pragma once
#include "Core/Component.h"
#include <memory>

namespace Scythe
{
    class Model;
    class Shader;

    class MeshRendererComponent : public ComponentImpl<MeshRendererComponent>
    {
     SCYTHE_COMPONENT(MeshRendererComponent)   
    public:
        MeshRendererComponent() = default;

        explicit MeshRendererComponent(std::shared_ptr<Model> model);

        explicit MeshRendererComponent(Model&& model);

        void Draw(const std::shared_ptr<Shader>& shader) const;
    
    protected:
        void OnAttach(GameObject* owner) override;
    private:
        std::shared_ptr<Model> m_Model;
    };
}
