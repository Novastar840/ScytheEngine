#include <filesystem>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include "Model.h"
#include "Shader.h"
#include "Window.h"

#include "glm/fwd.hpp"
#include <glm/glm.hpp>                      // Core GLM (vec3, mat4)
#include <glm/gtc/matrix_transform.hpp>     // lookAt, perspective, rotate, translate
#include <glm/gtc/type_ptr.hpp>             // value_ptr

int main() {
    Scythe::Window window(800, 600, "Scythe Engine");
    
    spdlog::info("Working dir: {}", std::filesystem::current_path().string());
    
    Scythe::Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    //Scythe::Shader shader("assets/shaders/basic.vert", "assets/shaders/red.frag");
    
    Scythe::Model bunny("assets/models/stanford-bunny.obj");
    spdlog::info("Model loaded");
    
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.5f, 2.0f),  // Camera position
        glm::vec3(0.0f, 0.4f, 0.0f),  // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    ); 
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),     // Field of View
        800.0f / 600.0f,                // Aspect Ratio
        0.1f,                           // Near clipping plane
        100.0f                          // Far clipping plane
    );
    
    shader.Bind();
    shader.SetMat4("uView", glm::value_ptr(view));
    shader.SetMat4("uProjection", glm::value_ptr(projection));
    
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f); // dark blue-grey
    float angle = 0.0f;
    // Main loop
    while (!window.ShouldClose()) {
        window.Clear();
        shader.Bind();

        // Create Model Matrix (Rotate and scale the model)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.085f, 0.0f)); // Position
        
        
        angle += 0.01f;
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y axis
        
        model = glm::scale(model, glm::vec3(5.f)); 

        shader.SetMat4("uModel", glm::value_ptr(model));

        // Draw the loaded model
        bunny.Draw(shader);
        
        window.SwapBuffers();
        window.PollEvents();
    }

    return 0;
}
