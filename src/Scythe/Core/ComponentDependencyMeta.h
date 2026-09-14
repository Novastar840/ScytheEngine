#pragma once
#include <concepts>
#include <type_traits>

namespace Scythe
{
    class Component;
    // ----- Dependency management -----
    
    template <typename... Ts>
    struct StaticComponentsTag
    {
    };

    template <typename... Ts>
    struct TypeList
    {
    };

    template <typename, typename = void>
    struct ComponentRequires
    {
        using Type = TypeList<>;
    };

    template <typename T>
    struct ComponentRequires<T, std::void_t<typename T::Requires>>
    {
        using Type = T::Requires;
    };

    template <typename T>
        requires std::derived_from<T, Component>
    using ComponentRequiresList = ComponentRequires<T>::Type;

    // compile-time helpers

    template <typename T, typename List>
    struct TypeListContains;

    template <typename T, typename... Ts>
    struct TypeListContains<T, TypeList<Ts...>>
        : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
    {
    };

    template <typename List>
    struct AllDeriveFromComponent;

    template <typename... Ts>
    struct AllDeriveFromComponent<TypeList<Ts...>>
        : std::bool_constant<(std::derived_from<Ts, Component> && ...)>
    {
    };

    // ----- Circular dependency detection -----

    template <typename List, typename T>
    struct TypeListAppend;

    template <typename T, typename... Ts>
    struct TypeListAppend<TypeList<Ts...>, T>
    {
        using Type = TypeList<Ts..., T>;
    };

    template <typename List, typename T>
    using TypeListAppendT = TypeListAppend<List, T>::Type;

    template <typename List, typename Visited>
    struct DependencyCycleCheckList;

    template <bool Found, typename T, typename Visited>
    struct DependencyCycleCheckImpl;

    template <typename T, typename Visited>
    struct DependencyCycleCheckImpl<true, T, Visited>
    {
        static constexpr bool value = true;
    };

    template <typename T, typename Visited>
    struct DependencyCycleCheckImpl<false, T, Visited>
    {
        static constexpr bool value =
            DependencyCycleCheckList<ComponentRequiresList<T>, TypeListAppendT<Visited, T>>::value;
    };

    template <typename T, typename Visited>
    struct DependencyCycleCheck
    {
        static constexpr bool value =
            DependencyCycleCheckImpl<TypeListContains<T, Visited>::value, T, Visited>::value;
    };

    template <typename... Ts, typename Visited>
    struct DependencyCycleCheckList<TypeList<Ts...>, Visited>
    {
        static constexpr bool value = (DependencyCycleCheck<Ts, Visited>::value || ...);
    };

    template <typename T>
    using HasCircularDependency = DependencyCycleCheck<T, TypeList<>>;

    template <typename RequiredList, typename AvailableList>
    struct TypeListSubset;

    template <typename... RequiredTypes, typename AvailableList>
    struct TypeListSubset<TypeList<RequiredTypes...>, AvailableList>
        : std::bool_constant<(TypeListContains<RequiredTypes, AvailableList>::value && ...)>
    {
    };

    template <typename Pack>
    struct PackDependenciesSatisfied;

    template <typename... Ts>
    struct PackDependenciesSatisfied<TypeList<Ts...>>
        : std::bool_constant<(TypeListSubset<ComponentRequiresList<Ts>, TypeList<Ts...>>::value && ...)>
    {
    };
}
