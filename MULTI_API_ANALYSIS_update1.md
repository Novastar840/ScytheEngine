# Multi-API Refactor: Status Report (Update 1)

**Date**: 2026-06-29
**Status**: Early scaffolding — interfaces defined, zero implementations written. Codebase remains 100% OpenGL-coupled.

---

## Current State

Abstract interfaces exist in the codebase but have no implementations. The existing rendering pipeline uses raw OpenGL calls at every layer (Window → Shader → Mesh/Texture → Model). Vulkan support has not been started.

### What Exists (Scaffolding Only)

| Component | File | Status |
|-----------|------|--------|
| `RendererAPI` abstract base | `src/Scythe/Core/RendererAPI.h` | Interface defined, factory method unimplemented |
| `GraphicsContext` abstract base | `src/Scythe/Core/GraphicsContext.h` | Interface defined only (has constructor typo — should be destructor) |
| `VertexBuffer` / `IndexBuffer` interfaces | `src/Scythe/Core/Buffer.h` | Abstract classes with static `Create()` factories declared but **not implemented** |

### What Does NOT Exist

- No OpenGL implementation of `RendererAPI`, `GraphicsContext`, `VertexBuffer`, or `IndexBuffer`
- No Vulkan implementation of anything
- No `IShader`, `IMesh`, `ITexture` interfaces (the analysis doc proposes these but they were never created)
- No `RendererFactory` / API selection mechanism
- No Vulkan SDK dependency in CMakeLists.txt

---

## OpenGL Coupling Inventory

Every rendering layer directly uses GL functions:

| File | Coupling Points |
|------|----------------|
| `Shader.h/cpp` | `glCreateProgram`, `glAttachShader`, `glLinkProgram`, `glGetProgramiv`, `glDeleteShader`, `glUseProgram`, `glGetUniformLocation`, `glUniform1i/f/matrix4fv` |
| `Mesh.h/cpp` | `glGenVertexArrays`, `glBindVertexArray`, `glGenBuffers`, `glBindBuffer`, `glBufferData`, `glEnableVertexAttribArray`, `glVertexAttribPointer`, `glDrawElements`, `glActiveTexture`, `glBindTexture` |
| `Model.cpp` | `glGenTextures`, `glBindTexture`, `glTexImage2D`, `glGenerateMipmap`, `glTexParameteri` (in `TextureFromFile()`) |
| `Window.cpp` | `glfwWindowHint(GLFW_OPENGL_PROFILE)`, `gladLoadGL`, `glViewport`, `glEnable(GL_DEPTH_TEST)` |
| `Main.cpp` | Direct `glClearColor()` call, includes `<glad/gl.h>` |

---

## What Still Needs to Be Done

### Phase 1: Implement OpenGL Backend (Prerequisite — must come first)

The interfaces are defined but have no implementations. Before Vulkan can be added, the existing OpenGL code must be wrapped in these interfaces so it compiles through them.

**Priority order:**

1. **`OpenGLGraphicsContext`** — wraps GLFW window + GLAD context creation
2. **`OpenGLVertexBuffer` / `OpenGLIndexBuffer`** — wrap VBO/EBO with `Bind()`/`Unbind()`
3. **`OpenGLRendererAPI`** — implements `Init()`, `SetViewport()`, `SetClearColor()`, `Clear()`, `DrawIndexed()` using GL calls
4. **`IShader` interface + `OpenGLShader`** — extract current `Shader.h/cpp` into an implementation of this interface
5. **`ITexture` interface + `OpenGLTexture`** — wrap texture creation from `Model.cpp::TextureFromFile()`
6. **`IMesh` interface + `OpenGLMesh`** — refactor existing `Mesh.h/cpp` to implement `IMesh`
7. **`RendererFactory`** — static factory selecting between OpenGL/Vulkan backends

### Phase 2: Decouple Application Code from OpenGL

Once the OpenGL wrappers exist, update all callers:

- **`Main.cpp`**: Replace direct GL calls (`glClearColor`, etc.) with interface method calls through `RendererAPI::GetAPI()` or a renderer singleton
- **`Model.cpp`**: Replace `TextureFromFile()` returning raw IDs with factory-based texture creation returning `shared_ptr<ITexture>`
- **`Mesh.h/cpp`**: Update `Draw()` to accept the shader interface, not concrete class
- **`Window.cpp`**: Remove GL-specific code (`glViewport`, `glEnable(GL_DEPTH_TEST)`), keep GLFW windowing only

### Phase 3: Implement Vulkan Backend

This is a large independent body of work. The abstraction layer from Phase 1/2 must be complete before this can start meaningfully.

- **`VulkanGraphicsContext`** — manages VkInstance, VkPhysicalDevice, VkSurface
- **`VulkanRendererAPI`** — swapchain management, render passes, command buffer recording
- **`VulkanVertexBuffer/IndexBuffer`** — VkBuffer + VkDeviceMemory allocation
- **`VulkanShader`** — GLSL-to-SPIR-V compilation (glslangValidator), pipeline layout, descriptor sets, graphics pipeline creation
- **`VulkanTexture`** — VkImage + memory + view + sampler creation and descriptor updates
- **`VulkanMesh`** — buffer upload, vertex input state, draw command recording

### Phase 4: Build System & Integration

- Add Vulkan SDK as optional CMake dependency (`find_package(Vulkan)`)
- Add API selection mechanism (command-line arg or config file)
- Verify both backends produce identical visual output
- Fix the typo in `GraphicsContext.h` line 7 (`virtual GraphicsContext()` should be `virtual ~GraphicsContext()`)

---

## Estimated Scope

| Phase | New Files | Lines of Code | Difficulty |
|-------|-----------|---------------|------------|
| Phase 1: OpenGL wrappers | ~8 new files | +500 LOC, refactor existing 6 | Medium |
| Phase 2: Decouple app code | Modify existing 4-5 files | -300 / +200 LOC | Low-Medium |
| Phase 3: Vulkan backend | ~10 new files | +3000+ LOC | High |
| Phase 4: Build & integration | CMakeLists.txt, Main.cpp | +100 LOC | Medium |

**Key observation**: The multi-API analysis document (`MULTI_API_ANALYSIS.md`) is a thorough design spec. The actual implementation has barely begun — the interfaces are defined but entirely unimplemented. Phase 1 (OpenGL wrappers) should be treated as urgent since nothing else can proceed without it, and Phase 3 (Vulkan) represents the bulk of remaining work.
