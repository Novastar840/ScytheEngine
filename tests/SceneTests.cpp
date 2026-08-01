#include <catch2/catch_test_macros.hpp>

#include "Core/Scene.h"

using namespace Scythe;

TEST_CASE("Scene construction and object creation", "[Scene]")
{
    Scene* scene = new Scene("Test");
    
    GameObject* gameObject = scene->CreateGameObject("TestGameObject");
    REQUIRE(gameObject != nullptr);
    REQUIRE(scene->GetGameObjectCount() == 1);
    
    scene->DestroyGameObject(gameObject);
    REQUIRE(scene->GetGameObjectCount() == 0);
    
    delete scene;
}