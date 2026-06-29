# Multi-API Support Analysis for Scythe Engine

## Executive Summary

The Scythe Engine is currently **tightly coupled to OpenGL** with Vulkan support only partially defined in the enum. To enable runtime API switching between OpenGL and Vulkan, a comprehensive refactoring using **Abstraction Layer + Factory Pattern** is required. This document identifies all coupling points and provides a concrete implementation plan.

---

## Current Architecture Overview

```
Main.cpp (Entry Point)
    ↓
Window (GLFW Window Management + OpenGL Context Creation)
    ↓
Shader (OpenGL Shader Compilation & Program)
    ↓
Model → Mesh (Vertex Arrays, Buffers, Textures via GL calls)
    ↓
Camera (View/Projection Matrix Calculation - API Agnostic)
```

**Problem**: Every layer directly uses OpenGL functions (`glClear`, `glBindVertexArray`, `glCreateShader`, etc.) making it impossible to swap APIs without massive changes throughout the codebase.

---

## OpenGL Coupling Points Analysis

### 1. **Main.cpp** (Entry Point)
- **Line 2**: `#include <glad/gl.h>` - Direct GL header dependency
- **Line 35**: `glClearColor(0.2f, 0.2f, 0.3f, 1.0f)` - OpenGL state setting
- **Lines 57-60**: `shader.SetMat4(...)` - Direct API calls through shader
- **Line 62**: `bunny.Draw(shader)` - Rendering call tied to OpenGL mesh

**Impact**: Entry point is tightly coupled to OpenGL initialization and rendering pipeline.

---

### 2. **Window.h/cpp** (Window Management)
- **Lines 3, 6**: GLAD + GLFW includes for OpenGL context
- **Line 41**: `glfwMakeContextCurrent(m_Window)` - Creates OpenGL context
- **Lines 49-50**: `gladLoadGL(glfwGetProcAddress)` - GL function pointer loading
- **Line 56**: `glEnable(GL_DEPTH_TEST)` - OpenGL state configuration
- **Line 58**: `glViewport(0, 0, width, height)` - Viewport setup (Vulkan uses swapchain image)
- **Lines 29-31**: GLFW hints for OpenGL 3.3 Core Profile
- **Line 89**: `glClear(...)` in `Clear()` method

**Impact**: Window creation is fundamentally different between APIs:
- OpenGL: Uses GLFW window + GLAD context loading
- Vulkan: Requires instance, physical device, logical device, swapchain (no GLFW window)

---

### 3. **Shader.h/cpp** (Shader Management)
- **Line 2**: `#include <glad/gl.h>` - Direct GL dependency
- **Lines 14-15**: `GL_VERTEX_SHADER`, `GL_FRAGMENT_SHADER` - OpenGL enum constants
- **Line 17**: `glCreateProgram()` - Creates GL shader program
- **Lines 18-20**: `glAttachShader()`, `glLinkProgram()` - GL pipeline operations
- **Lines 25-28**: `glGetProgramiv()`, `glGetProgramInfoLog()` - GL error checking
- **Lines 32-33**: `glDeleteShader()` - GL resource cleanup
- **Line 43**: `glUseProgram(m_RendererID)` - Shader activation
- **Lines 51, 56, 60**: `glGetUniformLocation()`, `glUniform*()` - Uniform setting

**Impact**: Shader compilation and management differs drastically between APIs:
- OpenGL: GLSL shaders → Shader Program → Use Program
- Vulkan: SPIR-V bytecode → Pipeline Layout → Descriptor Sets → Graphics Pipeline

---

### 4. **Mesh.h/cpp** (Geometry & Rendering)
- **Line 4**: `#include <glad/gl.h>` - Direct GL dependency
- **Lines 34, 47**: `unsigned int m_VAO`, `m_VBO`, `m_EBO` - OpenGL resource IDs
- **Lines 103-128**: `SetupMesh()` uses:
  - `glGenVertexArrays()`, `glBindVertexArray()`
  - `glGenBuffers()`, `glBindBuffer()`
  - `glBufferData()` - Buffer upload (Vulkan uses VkBuffer + memory)
  - `glEnableVertexAttribArray()`, `glVertexAttribPointer()` - Vertex layout (Vulkan uses VkPipelineVertexInputStateCreateInfo)
- **Lines 78, 93, 94**: `glActiveTexture()`, `glBindVertexArray()`, `glDrawElements()` - Draw commands

**Impact**: Mesh representation is OpenGL-specific:
- VAO/VBO/EBO are GL concepts (Vulkan has no equivalent; vertex data lives in VkBuffer)
- Vertex attribute layout uses GL enums (Vulkan uses binding descriptions + attribute descriptions)
- Texture binding differs (GL: `glBindTexture()` vs Vulkan: descriptor sets)

---

### 5. **Model.cpp** (Asset Loading & Textures)
- **Line 8**: `#include <glad/gl.h>` - Direct GL dependency
- **Lines 21, 33-42**: `TextureFromFile()` uses OpenGL texture API:
  - `glGenTextures()`, `glBindTexture()`
  - `glTexImage2D()` - Image upload (Vulkan uses VkImage + staging buffer)
  - `glGenerateMipmap()` - Mip generation (OpenGL-only; Vulkan requires manual or external tool)
  - `glTexParameteri()` - Texture parameters

**Impact**: Texture creation and management is OpenGL-specific:
- GL: Single texture object with internal state
- Vulkan: VkImage + VkDeviceMemory + VkImageView + VkSampler (separate resources)

---

### 6. **RenderAPI.h/cpp** (Current State)
- **Lines 5-10**: Enum defines `OpenGL` and `Vulkan` but only OpenGL is implemented
- **Line 5 in RenderAPI.cpp**: Default is always OpenGL, no runtime switching works

**Impact**: The enum exists as a placeholder but the abstraction layer is incomplete.

---

## Required Abstraction Layer Design

### Core Interfaces to Create

#### 1. **IRenderer** (Top-level Renderer Interface)
```cpp
namespace Scythe {
    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        
        // Window/Context Management
        virtual void Initialize(int width, int height, const std::string& title) = 0;
        virtual void Shutdown() = 0;
        virtual bool ShouldClose() const = 0;
        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;
        
        // Frame Management
        virtual void BeginFrame() = 0;      // Clear, reset command buffer (Vulkan) / setup state (GL)
        virtual void EndFrame() = 0;        // Present (Vulkan) / swap buffers (GL)
        
        // State Management
        virtual void SetClearColor(float r, float g, float b, float a) = 0;
        virtual void Clear() = 0;           // glClear vs vkCmdBeginRenderPass/vkCmdEndRenderPass
        
        // Viewport
        virtual void SetViewport(int x, int y, int width, int height) = 0;
        
        // Depth Test
        virtual void EnableDepthTest(bool enable) = 0;
    };
}
```

#### 2. **IShader** (Shader Abstraction)
```cpp
namespace Scythe {
    class IShader {
    public:
        virtual ~IShader() = default;
        
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        
        // Uniform Setting (API-agnostic types)
        virtual void SetInt(const std::string& name, int value) const = 0;
        virtual void SetFloat(const std::string& name, float value) const = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) const = 0;
        
        // Shader Compilation (internal, called by factory)
        virtual bool Compile(const std::string& vertexSource, const std::string& fragmentSource) = 0;
    };
}
```

#### 3. **IMesh** (Geometry Abstraction)
```cpp
namespace Scythe {
    class IMesh {
    public:
        virtual ~IMesh() = default;
        
        virtual void Draw(const IShader& shader) const = 0;
        
        // Vertex Data Access (for loading/updates)
        virtual const std::vector<Vertex>& GetVertices() const = 0;
        virtual const std::vector<unsigned int>& GetIndices() const = 0;
        virtual const std::vector<Texture>& GetTextures() const = 0;
    };
}
```

#### 4. **ITexture** (Texture Abstraction)
```cpp
namespace Scythe {
    class ITexture {
    public:
        virtual ~ITexture() = default;
        
        virtual void Bind(uint32_t unit) const = 0;
        virtual void Unbind() const = 0;
        
        // Image Upload (internal, called by factory)
        virtual bool LoadFromFile(const std::string& path) = 0;
    };
}

struct Texture {
    std::shared_ptr<ITexture> texture;  // Polymorphic instead of unsigned int ID
    std::string Type;
    std::string Path;
};
```

#### 5. **IResourceManager** (Resource Management)
```cpp
namespace Scythe {
    class IResourceManager {
    public:
        virtual ~IResourceManager() = default;
        
        virtual std::shared_ptr<ITexture> LoadTexture(const std::string& path) = 0;
        virtual std::shared_ptr<IMesh> CreateMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            const std::vector<Texture>& textures
        ) = 0;
    };
}
```

---

## Factory Pattern Implementation

### **RendererFactory** (API Selection)
```cpp
namespace Scythe {
    class RendererFactory {
    public:
        static std::unique_ptr<IRenderer> Create(RenderAPI api);
        
        // Helper to get current renderer
        static IRenderer& GetCurrent() {
            return *s_Current;
        }
        
    private:
        static std::unique_ptr<IRenderer> s_Current;
    };
}
```

**Implementation**:
- `RenderAPI::OpenGL` → Returns `OpenGLRenderer` (wraps GLFW + GLAD)
- `RenderAPI::Vulkan` → Returns `VulkanRenderer` (manages Vulkan instance/device/swapchain)

---

## CMake Configuration Changes Required

### Current:
```cmake
add_library(glad STATIC vendor/glad/src/gl.c)
target_include_directories(glad PUBLIC vendor/glad/include)
target_link_libraries(EngineApp PRIVATE glfw glad spdlog::spdlog glm::glm assimp)
```

### Required:
```cmake
# OpenGL (GLFW + GLAD) - Always included for fallback/tooling
add_library(glad STATIC vendor/glad/src/gl.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# Vulkan SDK (optional, conditionally compiled)
find_package(Vulkan REQUIRED)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(EngineApp PRIVATE 
        glfw glad vulkan-1 spdlog::spdlog glm::glm assimp
    )
endif()
```

**Note**: Vulkan SDK must be installed on the build machine. GLAD can remain always-included since OpenGL is used for development tools even if runtime uses Vulkan.

---

## Refactoring Plan (Step-by-Step)

### Phase 1: Define Interfaces (No Behavioral Changes)
**Goal**: Create abstraction layer without breaking existing code.

1. **Create `IRenderer` interface** in `include/Renderer/IRenderer.h`
2. **Create `IShader` interface** in `include/Shader/IShader.h`
3. **Create `IMesh` interface** in `include/Mesh/IMesh.h`
4. **Create `ITexture` interface** in `include/Texture/ITexture.h`
5. **Update `Texture` struct** to use `shared_ptr<ITexture>` instead of `unsigned int ID`

**Files Modified**:
- `src/Mesh.h` - Update Texture struct definition
- All files using `Texture.ID` need updating

---

### Phase 2: Implement OpenGL Wrappers (Existing Code Refactored)
**Goal**: Make existing OpenGL code work through interfaces.

1. **Create `OpenGLRenderer`** class implementing `IRenderer`:
   - Wraps current `Window.cpp` functionality
   - Manages GLFW window + GLAD context
   
2. **Create `OpenGLShader`** class implementing `IShader`:
   - Refactor current `Shader.h/cpp` into this class
   
3. **Create `OpenGLMesh`** class implementing `IMesh`:
   - Refactor current `Mesh.h/cpp` into this class
   
4. **Create `OpenGLTexture`** class implementing `ITexture`:
   - Extract `TextureFromFile()` logic from `Model.cpp`

5. **Update `RendererFactory`**:
   - Implement creation of OpenGL implementations

---

### Phase 3: Update Existing Code to Use Interfaces
**Goal**: Decouple existing code from concrete OpenGL classes.

1. **Update `Main.cpp`**:
   ```cpp
   // Before: Direct API usage
   glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
   
   // After: Through interface
   IRenderer::GetCurrent().SetClearColor(0.2f, 0.2f, 0.3f, 1.0f);
   ```

2. **Update `Model.cpp`**:
   - Replace direct GL texture calls with `ITexture` interface usage
   - Update `TextureFromFile()` to return `shared_ptr<ITexture>`

3. **Update `Mesh.h/cpp`**:
   - Store textures as `vector<Texture>` where Texture contains `shared_ptr<ITexture>`
   - `Draw()` takes `const IShader&` instead of concrete class

4. **Update `Window.h/cpp`**:
   - Either remove (merge into OpenGLRenderer) or keep as GLFW wrapper only
   - Remove GL-specific calls (`glViewport`, `glEnable(GL_DEPTH_TEST)`)

---

### Phase 4: Implement Vulkan Backend (New Feature)
**Goal**: Add Vulkan rendering support.

1. **Create `VulkanRenderer`** class implementing `IRenderer`:
   - Manages VkInstance, VkPhysicalDevice, VkDevice
   - Creates swapchain, render passes, framebuffers
   - Handles command buffer recording and submission
   
2. **Create `VulkanShader`** class implementing `IShader`:
   - Compiles GLSL → SPIR-V (using glslangValidator or runtime compiler)
   - Creates VkPipelineLayout, VkDescriptorSetLayout
   - Builds VkGraphicsPipeline
   
3. **Create `VulkanMesh`** class implementing `IMesh`:
   - Creates VkBuffer for vertex/index data
   - Allocates VkDeviceMemory
   - Records draw commands into command buffers
   
4. **Create `VulkanTexture`** class implementing `ITexture`:
   - Creates VkImage, allocates memory, creates view/sampler
   - Handles image transitions and descriptor updates

---

### Phase 5: Integration & Testing
**Goal**: Ensure both APIs work correctly with runtime switching.

1. **Update `Main.cpp` entry point**:
   ```cpp
   int main() {
       auto renderer = RendererFactory::Create(RenderAPI::OpenGL); // or Vulkan
       
       renderer->Initialize(800, 600, "Scythe Engine");
       
       // ... rest of code uses interfaces
       
       return 0;
   }
   ```

2. **Add API selection configuration**:
   - Command-line argument: `--api vulkan` or `--api opengl`
   - Configuration file: `config.json` with `"renderer": "vulkan"`
   - Runtime toggle (for debugging): Press key to switch APIs

3. **Test both APIs**:
   - Verify identical visual output
   - Check performance characteristics
   - Ensure no API-specific leaks or hangs

---

## Key Technical Challenges

### 1. **Texture Management Complexity**
- **OpenGL**: Single object with internal state (bound texture, parameters)
- **Vulkan**: Separate resources (Image + Memory + View + Sampler) requiring descriptor set updates
- **Solution**: Abstract behind `ITexture` interface; Vulkan implementation handles all complexity internally

### 2. **Command Buffer vs Immediate Mode**
- **OpenGL**: Immediate mode rendering (draw calls happen immediately)
- **Vulkan**: Command buffer recording → submission pattern
- **Solution**: 
  - OpenGL: Record into a command buffer then flush at `EndFrame()`
  - Vulkan: Already uses command buffers natively

### 3. **Synchronization**
- **OpenGL**: Implicit synchronization (driver handles it)
- **Vulkan**: Explicit synchronization (semaphores, fences, image layout transitions)
- **Solution**: Abstract into `IRenderer::BeginFrame()` / `EndFrame()` - Vulkan implementation handles all sync

### 4. **Shader Language Differences**
- **OpenGL**: GLSL shaders compiled at runtime
- **Vulkan**: SPIR-V bytecode (precompiled or compiled via glslang)
- **Solution**: 
  - Keep GLSL source in assets (cross-compatible)
  - Vulkan backend uses glslangValidator to compile GLSL → SPIR-V at load time

### 5. **Window Management**
- **OpenGL**: GLFW window + OpenGL context
- **Vulkan**: VK_KHR_surface extension for window integration, but no OpenGL context needed
- **Solution**: 
  - Keep GLFW for windowing (platform abstraction)
  - OpenGL uses GLAD for function loading
  - Vulkan uses `vkCreateInstance()` with `VK_KHR_SURFACE_EXTENSION_NAME`

---

## Header File Structure Recommendation

```
include/ScytheEngine/
├── Core/
│   ├── IRenderer.h          # Top-level renderer interface
│   ├── RendererFactory.h    # API selection factory
│   └── Types.h              # Common types (Vertex, Texture, etc.)
├── Shader/
│   ├── IShader.h            # Shader abstraction
│   ├── OpenGLShader.h       # OpenGL implementation
│   └── VulkanShader.h       # Vulkan implementation
├── Mesh/
│   ├── IMesh.h              # Mesh abstraction
│   ├── OpenGLMesh.h         # OpenGL implementation
│   └── VulkanMesh.h         # Vulkan implementation
├── Texture/
│   ├── ITexture.h           # Texture abstraction
│   ├── OpenGLTexture.h      # OpenGL implementation
│   └── VulkanTexture.h      # Vulkan implementation
└── Window/
    └── Window.h             # GLFW window wrapper (platform only)

src/Renderer/
├── OpenGL/
│   ├── OpenGLRenderer.cpp
│   ├── OpenGLShader.cpp
│   ├── OpenGLMesh.cpp
│   └── OpenGLTexture.cpp
├── Vulkan/
│   ├── VulkanRenderer.cpp
│   ├── VulkanShader.cpp
│   ├── VulkanMesh.cpp
│   └── VulkanTexture.cpp
└── RendererFactory.cpp

src/ (legacy, will be phased out)
├── Main.cpp                 # Updated to use interfaces
├── Model.cpp                # Updated to use ITexture
├── Camera.h/cpp             # Unchanged (matrix math only)
└── GameObject.h/cpp         # Unchanged (transform logic only)
```

---

## Migration Strategy

### Option A: **Big Bang Refactor** (Recommended for small codebase)
- Rewrite everything in one pass
- Pros: Clean architecture from start, no legacy GL calls scattered
- Cons: Longer development time, harder to test incrementally

### Option B: **Incremental Migration** (Safer approach)
1. Create interfaces and factories first
2. Implement OpenGL wrappers that delegate to existing code
3. Test that everything still works with OpenGL through interfaces
4. Implement Vulkan backend in parallel
5. Switch default API to Vulkan when ready
6. Remove legacy GL calls

### Option C: **Hybrid Approach** (Balanced)
- Phase 1-2 as described above (interfaces + OpenGL wrappers)
- Phase 3 updates existing code to use interfaces
- Phase 4 adds Vulkan in parallel while OpenGL still works
- Phase 5 integration and testing

---

## Estimated Complexity by Component

| Component | Files Affected | Lines of Code Change | Difficulty |
|-----------|---------------|---------------------|------------|
| Interfaces & Factory | New: ~20 files, Existing: ~10 | +1500 LOC, -200 LOC | Medium |
| OpenGL Wrappers | Refactor existing 6 files | -800 LOC (refactored) | Low-Medium |
| Vulkan Backend | New: ~20 files | +3000 LOC | High |
| Integration & Testing | Update Main.cpp, Model.cpp | +100 LOC | Medium |

**Total Estimated Effort**: 4000+ lines of new code, significant refactoring of existing OpenGL code.

---

## Conclusion

The current Scythe Engine is **tightly coupled to OpenGL** across all rendering layers (Window → Shader → Mesh/Texture → Model). Supporting Vulkan requires implementing an abstraction layer using interfaces and a factory pattern for API selection.

**Key Takeaways**:
1. Create `IRenderer`, `IShader`, `IMesh`, `ITexture` interfaces first
2. Wrap existing OpenGL code in OpenGL implementations of these interfaces
3. Use `RendererFactory` to select API at startup
4. Implement Vulkan backend separately (can be done in parallel)
5. Keep Camera and GameObject layers unchanged (they're already API-agnostic)

The `RenderAPI` enum already exists as a placeholder - the work is to make it functional by implementing the abstraction layer around it.
