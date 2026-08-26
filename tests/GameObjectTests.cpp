#include <catch2/catch_test_macros.hpp>
#include "Core/GameObject.h"
#include "Components/TransformComponent.h"
//#include "Core/Component.h"

using namespace Scythe;

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