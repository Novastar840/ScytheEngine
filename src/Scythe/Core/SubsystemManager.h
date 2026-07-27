#pragma once
#include "Subsystem.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <utility>

namespace Scythe 
{
    class SubsystemManager 
    {
    public:
        template<typename T, typename... Args>
        T* Register(Args&&... args) 
        {
            uintptr_t typeID = GetTypeID<T>();
            
            // Prevent duplicate registrations
            if (m_Subsystems.contains(typeID)) 
            {
                return static_cast<T*>(m_Subsystems[typeID].get());
            }
            
            auto subsystem = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = subsystem.get();
            
            m_Subsystems[typeID] = std::move(subsystem);
            m_InitOrder.push_back(ptr);
            
            return ptr;
        }
        
        template<typename T>
        T* Get() const 
        {
            size_t typeID = GetTypeID<T>();
            auto it = m_Subsystems.find(typeID);
            if (it != m_Subsystems.end()) 
            {
                return static_cast<T*>(it->second.get());
            }
            return nullptr;
        }
        
        void InitializeAll() const
        {
            for (auto* sub : m_InitOrder) 
            {
                std::cout << "[Scythe] Initializing: " << sub->GetName() << "\n";
                if (!sub->Initialize()) 
                {
                    std::cerr << "[Scythe] FATAL: Failed to initialize " << sub->GetName() << "\n";
                }
            }
        }
        
        void ShutdownAll() 
        {
            for (auto it = m_InitOrder.rbegin(); it != m_InitOrder.rend(); ++it) 
            {
                std::cout << "[Scythe] Shutting down: " << (*it)->GetName() << "\n";
                (*it)->Shutdown();
            }
            m_Subsystems.clear();
            m_InitOrder.clear();
        }
        
        void Update(float deltaTime) const
        {
            for (auto subsystem : m_InitOrder)
            {
                subsystem->Update(deltaTime);
            }
        }
    private:
        template<typename T>
        static uintptr_t GetTypeID() 
        {
            static constexpr bool dummy = false;
            return reinterpret_cast<uintptr_t>(&dummy);
        }

        std::unordered_map<uintptr_t, std::unique_ptr<Subsystem>> m_Subsystems;
        std::vector<Subsystem*> m_InitOrder;
    };
}