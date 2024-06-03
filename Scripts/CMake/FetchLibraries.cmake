cmake_minimum_required(VERSION 3.26)

include(FetchContent)

# STB_IMAGE library
FetchContent_Declare(
    stbi
    GIT_REPOSITORY https://github.com/nothings/stb.git
)

#ASSIMP library
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
)

# IMGUI library
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)

# IMGUIZMO library
FetchContent_Declare(
	imguizmo
	GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG "1.83"
)

# YAML-CPP library
FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)

# SPIRV reflect
FetchContent_Declare(
    spirv-reflect
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG main
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Assimp build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Assimp install" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Assimp install PDB" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Assimp warnings as errors" FORCE)
set(SPIRV_REFLECT_STATIC_LIB TRUE)

