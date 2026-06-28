cmake_minimum_required(VERSION 3.20)
project(ScytheEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "" FORCE)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE ENGINE_SOURCES
        CONFIGURE_DEPENDS
        "src/*.cpp"
        "src/*.h"
)

add_executable(EngineApp ${ENGINE_SOURCES})
target_compile_options(EngineApp PRIVATE /W4)
target_compile_definitions(EngineApp PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

include(FetchContent)

FetchContent_Declare(
        glfw 
        GIT_REPOSITORY https://github.com/glfw/glfw.git 
        GIT_TAG 3.3.8)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v5.4.3 # Pin to a specific stable version!
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # Static linking is usually better for custom engines

FetchContent_MakeAvailable(glfw glm assimp)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.11.0)
FetchContent_MakeAvailable(spdlog)

add_library(glad STATIC vendor/glad/src/gl.c)
target_include_directories(glad PUBLIC vendor/glad/include)

target_include_directories(EngineApp PUBLIC vendor/stb/include)

target_link_libraries(EngineApp PRIVATE glfw glad spdlog::spdlog glm::glm assimp)

# Copy assets folder to the executable directory on every build
add_custom_target(CopyAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:EngineApp>/assets"
        COMMENT "Copying assets to build directory"
)
add_dependencies(EngineApp CopyAssets)