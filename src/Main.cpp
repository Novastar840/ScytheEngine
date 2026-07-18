#include <filesystem>
#include <spdlog/spdlog.h>

#include "Model.h"
#include "Window.h"
#include "Camera.h"
#include "GameObject.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scythe/Core/RendererAPI.h"
#include "Scythe/Core/Shader.h"

int main()
{
	Scythe::Window window(800, 600, "Scythe Engine");
	Scythe::RendererAPI::Initialize();
	
	Scythe::Camera camera("MainCamera", glm::vec3(0.0f, 0.5f, 2.0f));

	camera.SetPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

	camera.LookAtRotation(glm::vec3(0.0f, 0.4f, 0.0f));

	window.SetMainCamera(&camera);

	spdlog::info("Working dir: {} ", std::filesystem::current_path().string());

	auto shader = Scythe::Shader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag");
	shader->Bind();

	Scythe::Model bunny("assets/models/stanford-bunny.obj", "StanfordBunny", glm::vec3(0.0f, -0.085f, 0.0f));
	bunny.SetScale(5.f);
	spdlog::info("Model loaded");
	
	float bunnyYaw = 0.0f;
	
	Scythe::RendererAPI::Get()->SetClearColor(glm::vec4(0.2f, 0.2f, 0.3f, 1.0f));
	
	while (!window.ShouldClose())
	{
		static float lastFrame = 0.0;
		float currentFrame = window.GetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		Scythe::RendererAPI::Get()->Clear();
		
		bunnyYaw += 60.0f * deltaTime;
		bunny.SetRotation(glm::vec3(0.0f, bunnyYaw, 0.0f));

		camera.Tick(deltaTime);
		
		shader->SetMat4("uView", glm::value_ptr(window.GetMainCamera()->GetViewMatrix()));
		shader->SetMat4("uProjection", glm::value_ptr(window.GetMainCamera()->GetProjectionMatrix()));

		shader->SetMat4("uModel", glm::value_ptr(bunny.GetTransformMatrix()));

		bunny.Draw(shader);

		window.SwapBuffers();
		window.PollEvents();
	}

	return 0;
}
