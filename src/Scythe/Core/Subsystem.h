#pragma once

#include <functional>

namespace Scythe
{

class Subsystem
{
public:
    virtual ~Subsystem() = default;

    virtual bool Initialize() { return true; }
    virtual void Shutdown() {}
    virtual void Update(float deltaTime) {}

    virtual const char* GetName() const = 0;
};
    
// Stable type ID for subsystems.
// SubsystemManager and SCYTHE_SUBSYSTEM must use the same ID mechanism.
template <typename T>
uintptr_t SubsystemTypeID()
{
    static const char id = 0;
    return reinterpret_cast<uintptr_t>(&id);
}
    
// Auto-registration registry.
// Each SCYTHE_SUBSYSTEM(T) creates a static registrar that pushes T's
// factory into this vector before main() starts.
using SubsystemFactory = std::function<std::unique_ptr<Subsystem>()>;

struct SubsystemRegistryEntry
{
    const char* name;
    uintptr_t typeID;
    SubsystemFactory factory;
};

inline std::vector<SubsystemRegistryEntry>& GetSubsystemRegistry()
{
    static std::vector<SubsystemRegistryEntry> registry;
    return registry;
}

// ========================================================================
// Macro: declares and auto-registers a subsystem.
//
// Usage:
//     class RuntimeManager : public Subsystem
//     {
//         SCYTHE_SUBSYSTEM(RuntimeManager)
//
//     public:
//         ...
//     };
//
// ========================================================================
#define SCYTHE_SUBSYSTEM(Type)                                                       \
    static std::unique_ptr<::Scythe::Subsystem> Type##_Factory()                     \
    {                                                                                \
        return std::make_unique<Type>();                                             \
    }                                                                                \
                                                                                     \
    struct Type##_Registrar                                                          \
    {                                                                                \
        Type##_Registrar()                                                           \
        {                                                                            \
            ::Scythe::GetSubsystemRegistry().push_back(                              \
                ::Scythe::SubsystemRegistryEntry{                                    \
                    #Type,                                                           \
                    ::Scythe::SubsystemTypeID<Type>(),                               \
                    &Type##_Factory});                                               \
        }                                                                            \
    };                                                                               \
                                                                                     \
    static inline Type##_Registrar g_##Type##_Registrar{};

}