#include "RuntimeManager.h"

namespace Scythe
{
    bool RuntimeManager::Initialize()
    {
        return true;
    }

    void RuntimeManager::AddScene(std::unique_ptr<Scene> scene)
    {
        m_Scenes.push_back(std::move(scene));
    }

    void RuntimeManager::RemoveScene(const Scene* target)
    {
        for (auto& scene : m_Scenes)
        {
            if (scene.get() == target)
            {
                std::swap(scene, m_Scenes.back());
                m_Scenes.pop_back();
                return;
            }
        }
    }

    Scene* RuntimeManager::CreateScene(const std::string& name)
    {
        auto newScene = std::make_unique<Scene>(name);
        Scene* ptr = newScene.get();
        AddScene(std::move(newScene));
        return ptr;
    }
}
