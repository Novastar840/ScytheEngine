#pragma once

// ==========================================
// 1. Standard Library
// ==========================================
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <array>

// ==========================================
// 2. Third-Party Libraries
// ==========================================
// NOTE: glad MUST be included before glfw3.h to prevent OpenGL header conflicts!
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

// Assimp is usually only needed in the Mesh/Model loader, 
// so you might want to leave it out of the PCH to save compile time, 
// but if you want it everywhere, include it here:
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

// ==========================================
// 3. Engine Core Type Aliases
// ==========================================
namespace Scythe {
    // Math Types
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;
    using Mat4 = glm::mat4;
    using Quat = glm::quat;

    // Smart Pointers
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;
}