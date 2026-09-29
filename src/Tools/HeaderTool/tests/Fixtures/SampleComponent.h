#pragma once
#include "Reflection/ReflectionMacros.h"

namespace Scythe
{
    SCYTHE_CLASS(Category = "Test")
    class SampleComponent
    {
        GENERATED_BODY()
    public:
        SCYTHE_PROPERTY(Tooltip = "Move speed")
        float speed = 1.0f;

        SCYTHE_PROPERTY(Category = "Physics")
        int health = 100;

        float notReflected = 0.0f;
    };
}
