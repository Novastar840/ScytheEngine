#include <filesystem>

#include "Model.h"
#include "Core/Window.h"
#include "GameObject.h"

#include <glm/gtc/type_ptr.hpp>

#include "Components/CameraComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/TransformComponent.h"
#include "Core/RendererAPI.h"
#include "Core/Shader.h"
#include "Core/SubsystemManager.h"
#include "Subsystems/RuntimeManager.h"

using namespace Scythe;

static GameObject& SetupCamera()
{
	GameObject* camera = new GameObject("Camera");
		
	auto& camTransformComponent = camera->AddComponent<TransformComponent>(Vec3(0.0f, 0.5f, 2.0f));
	camTransformComponent.LookAtRotation(Vec3(0.0f, 0.4f, 0.0f));
		
	auto& camComp = camera->AddComponent<CameraComponent>();
	camComp.SetPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
	
	return *camera;
}

int main()
{
	try 
	{
		std::unique_ptr<SubsystemManager> subsystemManager = std::make_unique<SubsystemManager>();
		subsystemManager->RegisterAllFromRegistry();
		subsystemManager->InitializeAll();
		
		RendererAPI::SetAPI(RendererAPI::API::OpenGL);
	
		auto window = Window::Create("EngineApp", 800, 600);
		RendererAPI::Initialize();

		spdlog::info("Working dir: {} ", std::filesystem::current_path().string());
		
		GameObject& camera = SetupCamera();
		CameraComponent* camComp = camera.GetComponent<CameraComponent>();
		
		auto shader = Shader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag");
		shader->Bind();

		GameObject bunny("StanfordBunny");
        
		auto& bunnyTransform = bunny.AddComponent<TransformComponent>(Vec3(0.0f, -0.085f, 0.0f));
		bunnyTransform.SetScale(5.f);

		Model bunnyModel("assets/models/stanford-bunny.obj", "StanfordBunny");
        
		bunny.AddComponent<MeshRendererComponent>(std::move(bunnyModel));
		spdlog::info("Model loaded");
	
		RendererAPI::Get()->SetClearColor(Vec4(0.2f, 0.2f, 0.3f, 1.0f));
		
		window->SetMainCamera(camera.GetComponent<CameraComponent>());
		
		float lastFrame = 0.0;
		float bunnyYaw = 0.0f;
		
		while (!window->ShouldClose())
		{
			float currentFrame = window->GetTime();
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			
			subsystemManager->Update(deltaTime);
			
			RendererAPI::Get()->Clear();
		
			bunnyYaw += 60.0f * deltaTime;
			bunnyTransform.SetRotation(Vec3(0.0f, bunnyYaw, 0.0f));
			
			Mat4 viewMatrix = camComp->GetViewMatrix(*camera.GetComponent<TransformComponent>());
			Mat4 projMatrix = camComp->GetProjectionMatrix();

			shader->SetMat4("uView", glm::value_ptr(viewMatrix));
			shader->SetMat4("uProjection", glm::value_ptr(projMatrix));

			auto* renderer = bunny.GetComponent<MeshRendererComponent>();
			if (renderer) {
				renderer->Draw(shader);
			}

			window->SwapBuffers();
			window->PollEvents();
		}
		
		subsystemManager->ShutdownAll();
	}
	catch (const std::exception& e)
	{
		spdlog::critical("Engine Initialization Failed: {}", e.what());
		return -1;
	}
	
	return 0;
}
