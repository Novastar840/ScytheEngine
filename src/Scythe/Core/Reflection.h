// Reflection.h
#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <string_view>

namespace Scythe::Reflect
{
    template<typename T>
    uintptr_t TypeID()
    {
        static const char id = 0;
        return reinterpret_cast<uintptr_t>(&id);
    }

    template<typename Base>
    struct TypeInfo
    {
        const char* Name;
        uintptr_t   TypeID;
        std::function<std::unique_ptr<Base>()> Factory;
    };

    template<typename Base>
    std::vector<TypeInfo<Base>>& GetRegistry()
    {
        static std::vector<TypeInfo<Base>> registry;
        return registry;
    }

    template<typename Base>
    const TypeInfo<Base>* FindByName(std::string_view name)
    {
        for (auto& entry : GetRegistry<Base>())
            if (name == entry.Name) return &entry;
        return nullptr;
    }

    template<typename Base>
    std::unique_ptr<Base> CreateByName(std::string_view name)
    {
        if (auto* info = FindByName<Base>(name))
            return info->Factory();
        return nullptr;
    }
    
#define SCYTHE_REGISTER_IMPL(Base, Type)                                     \
    struct Type##_Registrar                                                  \
    {                                                                        \
        Type##_Registrar()                                                   \
        {                                                                    \
            ::Scythe::Reflect::GetRegistry<Base>().emplace_back(             \
                ::Scythe::Reflect::TypeInfo<Base>{                           \
                    #Type,                                                   \
                    ::Scythe::Reflect::TypeID<Type>(),                       \
                    [] { return std::make_unique<Type>(); }                  \
                });                                                          \
        }                                                                    \
    };                                                                       \
    static inline Type##_Registrar g_##Type##_Registrar{};
}

namespace Scythe
{
#define SCYTHE_COMPONENT(Type) SCYTHE_REGISTER_IMPL(Scythe::Component, Type)
#define SCYTHE_GAME_OBJECT(Type) SCYTHE_REGISTER_IMPL(Scythe::GameObject, Type)
#define SCYTHE_SUBSYSTEM(Type) SCYTHE_REGISTER_IMPL(Scythe::Subsystem, Type)    \
    public:                                                                     \
        const char* GetName() const override { return #Type; }                  \
    private:
}