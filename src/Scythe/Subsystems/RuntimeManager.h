#pragma once

#include "Core/Subsystem.h"
#include "Core/Scene.h"

namespace Scythe
{
    class RuntimeManager : public Subsystem
    {
        SCYTHE_SUBSYSTEM(RuntimeManager)
    public:
        bool Initialize() override;
        
        void AddScene(std::unique_ptr<Scene> scene);
        void RemoveScene(const Scene* target);
        Scene* CreateScene(const std::string& name);
        
        const char* GetName() const override { return "RuntimeManager"; }
    private:
        std::vector<std::unique_ptr<Scene>> m_Scenes;
    };
}
