#include <catch2/catch_test_macros.hpp>
#include "Core/GameObject.h"
#include "Components/TransformComponent.h"
#include "Core/Timers.h"

using namespace Scythe;


namespace Test
{
    static int Print()
    {
        std::cout << "Hello World!" << std::endl;
        return 0;
    }
}

TEST_CASE("GameObject Construction and Properties", "[GameObject]")
{
    std::string name = "TestObject";
    GameObject gameObject = GameObject(name);
    
    REQUIRE(gameObject.GetName() == name);
    REQUIRE(gameObject.GetID() != GameObject::INVALID_ID);
    
    GameObject secondGameObject = GameObject("second", MakeComponent<TransformComponent>());
    REQUIRE(secondGameObject.GetID() != gameObject.GetID());
    REQUIRE(secondGameObject.GetComponent<TransformComponent>() != nullptr);
}

TEST_CASE("GameObject Components", "[GameObject]")
{
    std::string name = "TestObject";
    GameObject gameObject = GameObject(name);
    
    gameObject.AddComponent<TransformComponent>();
    REQUIRE(gameObject.GetComponent<TransformComponent>() != nullptr);
    gameObject.RemoveComponent<TransformComponent>();
    REQUIRE(gameObject.GetComponent<TransformComponent>() == nullptr);
}

TEST_CASE("GameObject timing", "[Timer]")
{
    TimingGuard timingGuard{};
    
    timingGuard.SetNow("GameObject Init");
    GameObject gameObject{"test"};
    timingGuard.PrintTime();
    
    timingGuard.SetNow("GameObject Heap Init");
    GameObject* gameObject2 = new GameObject("test");
    timingGuard.PrintTime();
    delete gameObject2;
    
    timingGuard.SetNow("SceneObject Init");
    SceneObject sceneObject{"test"};
    timingGuard.PrintTime();
    
    TimeFunction("Test", Test::Print);
}

