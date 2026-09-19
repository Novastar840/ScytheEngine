cmake_minimum_required(VERSION 3.21)
project(ScytheHeaderTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Dependency bootstrap ladder (design doc §8.4 / §12)
#   0. Manual override (-DSCYTHE_LLVM_ROOT / LLVM_DIR+Clang_DIR) → skip vcpkg
#   1. vcpkg present?                   else error + clone/bootstrap hints
#   2. vcpkg bootstrapped?              else error + bootstrap command hint
#   3. llvm installed (triplet)?        else error + `vcpkg install` command
#   4. [clang] feature libs exist?      else error + reinstall with feature hint
#   5. Wire prefix path, find_package(LLVM/Clang), version-pin warn
# This block never installs anything — only detects and instructs.
# ============================================================================

set(SCYTHE_LLVM_ROOT "" CACHE PATH "Manual override: prefix of an LLVM dev install (skips all vcpkg checks)")
set(SCYTHE_VCPKG_ROOT "" CACHE PATH "Root of an existing vcpkg checkout (auto-detected if empty)")
set(SCYTHE_VCPKG_TRIPLET "" CACHE STRING "vcpkg triplet to look in (auto-detected if empty)")
set(SCYTHE_LLVM_EXPECTED_VERSION "" CACHE STRING "Optional exact LLVM version pin; warns on mismatch")

function(scythe_fatal)
    string(JOIN "" _scythe_msg ${ARGN})
    message(FATAL_ERROR "\n[ScytheHeaderTool] ${_scythe_msg}\n")
endfunction()

# --- Step 0: manual escape hatches ------------------------------------------
set(SCYTHE_SKIP_VCPKG OFF)
if(SCYTHE_LLVM_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${SCYTHE_LLVM_ROOT}")
    message(STATUS "[SHT] Manual LLVM root: ${SCYTHE_LLVM_ROOT} (vcpkg checks skipped)")
    set(SCYTHE_SKIP_VCPKG ON)
elseif(DEFINED LLVM_DIR AND DEFINED Clang_DIR)
    message(STATUS "[SHT] LLVM_DIR/Clang_DIR supplied manually (vcpkg checks skipped)")
    set(SCYTHE_SKIP_VCPKG ON)
endif()

if(NOT SCYTHE_SKIP_VCPKG)
    # --- Step 1: locate vcpkg --------------------------------------------------
    set(SCYTHE_VCPKG_ROOT_EXPLICIT OFF)
    if(SCYTHE_VCPKG_ROOT)
        set(SCYTHE_VCPKG_ROOT_EXPLICIT ON)          # user passed -DSCYTHE_VCPKG_ROOT
    endif()

    # Filter out VS's private vcpkg under VC\vcpkg.
    # VS's bundled copy lives under Program Files and gets reset/relocated by
    # VS updates, so anything installed into it (like our LLVM build)
    # can silently vanish. We only ever want a user-managed, standalone vcpkg.
    function(scythe_vs_bundled root outvar)
        if(root MATCHES "Microsoft Visual Studio")
            set(${outvar} ON PARENT_SCOPE)
        else()
            set(${outvar} OFF PARENT_SCOPE)
        endif()
    endfunction()


    set(_via "none")   # filled by first detection branch that succeeds

    # 1a. Active vcpkg toolchain (authoritative for this build)
    if(DEFINED VCPKG_TOOLCHAIN)
        if(NOT SCYTHE_VCPKG_ROOT AND CMAKE_TOOLCHAIN_FILE)
            get_filename_component(_p "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
            get_filename_component(_p "${_p}/../.." ABSOLUTE)
            set(SCYTHE_VCPKG_ROOT "${_p}")
        endif()
        scythe_vs_bundled("${SCYTHE_VCPKG_ROOT}" _vs)
        if(_vs AND NOT SCYTHE_VCPKG_ROOT_EXPLICIT)
            scythe_fatal(
                "This configure runs with Visual Studio's BUNDLED vcpkg toolchain:\n"
                "    ${CMAKE_TOOLCHAIN_FILE}\n"
                "Installing LLVM there is fragile (Program Files; VS updates reset it).\n"
                "Reconfigure using your standalone vcpkg instead:\n"
                "    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake\n"
                "or drop the toolchain entirely - this ladder finds C:/vcpkg on its own.\n"
                "Also check for a CMAKE_TOOLCHAIN_FILE environment variable or a\n"
                "CMakePresets.json injecting the VS toolchain behind your back.")
        endif()
        set(SCYTHE_VCPKG_DETECTED ON)
        set(_via "active toolchain")
    endif()

    # 1b. VCPKG_ROOT env - also rejects VS-bundled and missing toolchain
    if(NOT SCYTHE_VCPKG_DETECTED AND DEFINED ENV{VCPKG_ROOT})
        scythe_vs_bundled("$ENV{VCPKG_ROOT}" _vs)
        if(NOT _vs AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
            set(SCYTHE_VCPKG_DETECTED ON)
            set(SCYTHE_VCPKG_ROOT "$ENV{VCPKG_ROOT}")
            set(_via "VCPKG_ROOT env")
        else()
            message(STATUS "[SHT] Ignoring VCPKG_ROOT env '$ENV{VCPKG_ROOT}' (VS-bundled or invalid)")
        endif()
    endif()

    # 1c. Well-known standalone locations (skipped only if already detected)
    if(NOT SCYTHE_VCPKG_DETECTED)
        set(_candidates
            "C:/vcpkg" "D:/vcpkg"
            "$ENV{USERPROFILE}/vcpkg" "$ENV{HOME}/vcpkg"
            "/opt/vcpkg" "/usr/local/vcpkg"
            "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
        foreach(_c IN LISTS _candidates)
            if(_c AND (EXISTS "${_c}/scripts/buildsystems/vcpkg.cmake"
                    OR EXISTS "${_c}/vcpkg.exe"
                    OR EXISTS "${_c}/vcpkg"))
                set(SCYTHE_VCPKG_DETECTED ON)
                set(SCYTHE_VCPKG_ROOT "${_c}")
                set(_via "candidate path")
                break()
            endif()
        endforeach()
    endif()

    # 1d. PATH search - VS-bundled result rejected
    if(NOT SCYTHE_VCPKG_DETECTED)
        find_program(SCYTHE_VCPKG_EXE NAMES vcpkg vcpkg.exe NO_CACHE)
        if(SCYTHE_VCPKG_EXE)
            get_filename_component(_r "${SCYTHE_VCPKG_EXE}" DIRECTORY)
            scythe_vs_bundled("${_r}" _vs)
            if(NOT _vs AND EXISTS "${_r}/scripts/buildsystems/vcpkg.cmake")
                set(SCYTHE_VCPKG_DETECTED ON)
                set(SCYTHE_VCPKG_ROOT "${_r}")
                set(_via "PATH search")
            endif()
        endif()
    endif()

    message(STATUS "[SHT] vcpkg detected at: ${SCYTHE_VCPKG_ROOT} (via ${_via})")

    # --- Step 2: bootstrapped? --------------------------------------------------
    if(WIN32 AND NOT EXISTS "${SCYTHE_VCPKG_ROOT}/vcpkg.exe")
        scythe_fatal(
                "vcpkg checkout found at ${SCYTHE_VCPKG_ROOT} but it is not bootstrapped.\n"
                "Run:  \"${SCYTHE_VCPKG_ROOT}/bootstrap-vcpkg.bat\"   (Linux/macOS: ./bootstrap-vcpkg.sh)\n"
                "Then reconfigure.")
    elseif(NOT WIN32 AND NOT EXISTS "${SCYTHE_VCPKG_ROOT}/vcpkg")
        scythe_fatal(
                "vcpkg checkout found at ${SCYTHE_VCPKG_ROOT} but it is not bootstrapped.\n"
                "Run:  ${SCYTHE_VCPKG_ROOT}/bootstrap-vcpkg.sh\n"
                "Then reconfigure.")
    endif()

    # --- Triplet resolution ------------------------------------------------------
    if(NOT SCYTHE_VCPKG_TRIPLET)
        if(DEFINED VCPKG_TARGET_TRIPLET)
            set(SCYTHE_VCPKG_TRIPLET "${VCPKG_TARGET_TRIPLET}")
        elseif(WIN32)
            set(SCYTHE_VCPKG_TRIPLET "x64-windows")
        elseif(APPLE)
            if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
                set(SCYTHE_VCPKG_TRIPLET "arm64-osx")
            else()
                set(SCYTHE_VCPKG_TRIPLET "x64-osx")
            endif()
        else()
            set(SCYTHE_VCPKG_TRIPLET "x64-linux")
        endif()
    endif()
    set(_vcpkg_prefix "${SCYTHE_VCPKG_ROOT}/installed/${SCYTHE_VCPKG_TRIPLET}")

    # --- Step 3: llvm package installed for this triplet? ------------------------
    if(NOT EXISTS "${_vcpkg_prefix}/share/llvm/copyright"
            AND NOT EXISTS "${_vcpkg_prefix}/lib/cmake/llvm/LLVMConfig.cmake"
            AND NOT EXISTS "${_vcpkg_prefix}/share/llvm/LLVMConfig.cmake")
        scythe_fatal(
                "vcpkg is installed, but llvm is NOT installed for triplet '${SCYTHE_VCPKG_TRIPLET}'.\n"
                "Missing marker: ${_vcpkg_prefix}/share/llvm/copyright\n"
                "\n"
                "Install it (one-time, heavy - builds LLVM from source):\n"
                "    \"${SCYTHE_VCPKG_ROOT}/vcpkg\" install \"llvm[clang]:${SCYTHE_VCPKG_TRIPLET}\"\n"
                "\n"
                "Budget ~1-3 h compile time and 30-60 GB of disk.\n"
                "After install, reclaim buildtrees (~20-40 GB):\n"
                "    \"${SCYTHE_VCPKG_ROOT}/vcpkg\" cleanup\n"
                "\n"
                "Then re-run this configure.")
    endif()

    # --- Step 4: clang LibTooling feature present? -------------------------------
    file(GLOB _tooling_lib "${_vcpkg_prefix}/lib/clangTooling*" )
    file(GLOB _tooling_lib2 "${_vcpkg_prefix}/lib/libclangTooling*")
    if(NOT _tooling_lib AND NOT _tooling_lib2)
        scythe_fatal(
                "llvm is installed for '${SCYTHE_VCPKG_TRIPLET}' but WITHOUT the clang feature\n"
                "(no clangTooling library under ${_vcpkg_prefix}/lib).\n"
                "Reinstall with the feature:\n"
                "    \"${SCYTHE_VCPKG_ROOT}/vcpkg\" install \"llvm[clang]:${SCYTHE_VCPKG_TRIPLET}\"")
    endif()

    # --- Step 5: make the installed tree discoverable ----------------------------
    list(APPEND CMAKE_PREFIX_PATH "${_vcpkg_prefix}")
    if(NOT DEFINED VCPKG_TOOLCHAIN)
        message(WARNING
                "[SHT] vcpkg packages found, but the vcpkg toolchain file is NOT active for this configure.\n"
                "Proceeding via CMAKE_PREFIX_PATH only. For full ABI/CRT consistency prefer:\n"
                "  -DCMAKE_TOOLCHAIN_FILE=${SCYTHE_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    endif()
endif() # NOT SCYTHE_SKIP_VCPKG

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
message(STATUS "[SHT] LLVM ${LLVM_PACKAGE_VERSION} resolved via ${LLVM_DIR}")

if(SCYTHE_LLVM_EXPECTED_VERSION AND NOT LLVM_PACKAGE_VERSION VERSION_EQUAL SCYTHE_LLVM_EXPECTED_VERSION)
    message(WARNING
            "[SHT] LLVM ${LLVM_PACKAGE_VERSION} != pinned ${SCYTHE_LLVM_EXPECTED_VERSION}.\n"
            "AST API drift is possible; test before adopting a new LLVM release.")
endif()

add_executable(ScytheHeaderTool src/Main.cpp)

target_include_directories(ScytheHeaderTool PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_compile_definitions(ScytheHeaderTool PRIVATE ${LLVM_DEFINITIONS})
target_compile_options(ScytheHeaderTool PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

target_link_libraries(ScytheHeaderTool PRIVATE
        clangTooling
        clangASTMatchers
        clangFrontend
        clangSerialization
        clangBasic
)