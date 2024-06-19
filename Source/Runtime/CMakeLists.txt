cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- DATA FETCHING -----------------------
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    get_filename_component(Vulkan_SDK_DIR "${Vulkan_LIBRARIES}" DIRECTORY)
    message(STATUS "Vulkan SDK found at: ${Vulkan_SDK_DIR}")
endif()
# ------------------------------------------------------------

# ---------------------- CMAKE FETCH -------------------------
FetchContent_MakeAvailable(stbi)
FetchContent_MakeAvailable(yaml-cpp)
FetchContent_MakeAvailable(spirv-reflect)
FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(glm)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Source/Runtime/*.cpp" "${CMAKE_SOURCE_DIR}/Source/Runtime/*.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Runtime" FILES ${SUBDIRECTORIES})
add_library(Runtime STATIC main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# ---------------------- IMGUI FILES -------------------------
file(GLOB IMGUI_FILES "${imgui_SOURCE_DIR}/*.cpp" "${imgui_SOURCE_DIR}/*.h")
source_group(TREE "${imgui_SOURCE_DIR}" FILES ${IMGUI_FILES})
add_library(imgui STATIC ${IMGUI_FILES})
# ------------------------------------------------------------

# ------------------- INCLUDE DIRECTORIES --------------------
target_include_directories(Runtime PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_include_directories(Runtime PUBLIC "${imgui_SOURCE_DIR}")
# ------------------------------------------------------------

# ------------------- LINK -----------------------------------
target_link_libraries(Runtime PUBLIC assimp)
target_link_libraries(Runtime PUBLIC yaml-cpp)
target_link_libraries(Runtime PUBLIC imgui)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/dxcompilerd.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/glslangd.lib")

    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/shaderc_combinedd.lib")

    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-cored.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-glsld.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-hlsld.lib")
else()
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/dxcompiler.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/glslang.lib")

    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/shaderc_combined.lib")

    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-core.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-glsl.lib")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/spirv-cross-hlsl.lib")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(Runtime PRIVATE Shlwapi.lib)
    target_link_libraries(Runtime PRIVATE Comctl32.lib)
    target_link_libraries(Runtime PRIVATE dinput8.lib)
    target_link_libraries(Runtime PRIVATE XInput.lib)
    target_link_libraries(Runtime PRIVATE dxguid.lib)
    target_link_libraries(Runtime PRIVATE Ws2_32.lib)
endif ()
# ------------------------------------------------------------

# ----------------- SPIRV-Reflect IMPLEMENTATION -------------
if(spirv-reflect_POPULATED)
	target_include_directories(Runtime PUBLIC ${spirv-reflect_SOURCE_DIR})
	target_link_libraries(Runtime PUBLIC spirv-reflect-static)
endif()
# ------------------------------------------------------------

# ----------------- STB IMAGE IMPLEMENTATION -----------------
if (stbi_POPULATED) 
    target_compile_definitions(Runtime PUBLIC STB_IMAGE_IMPLEMENTATION=1)
    target_include_directories(Runtime PUBLIC ${stbi_SOURCE_DIR})
else ()
    target_compile_definitions(Runtime PUBLIC STB_IMAGE_IMPLEMENTATION=0)
endif ()
# ------------------------------------------------------------

# ----------------- GLM IMPLEMENTATION -----------------------
if(glm_POPULATED)
	target_include_directories(Runtime PUBLIC ${glm_SOURCE_DIR})
	target_link_libraries(Runtime PUBLIC glm)
endif()
# ------------------------------------------------------------

# ------------------- VULKAN IMPLEMENTATION ------------------
if(Vulkan_FOUND)
    target_include_directories(Runtime PUBLIC "${Vulkan_INCLUDE_DIRS}")
    target_link_libraries(Runtime PUBLIC "${Vulkan_SDK_DIR}/vulkan-1.lib")
endif()
# ------------------------------------------------------------


# ----------------- PREPROCESSOR DEFINITIONS -----------------
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_DEBUG)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_RELEASE)
endif ()
# ------------------------------------------------------------

# ------------------ PLATFORM DEFINITIONS --------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_WINDOWS=1)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_LINUX=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_MACOS=0)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND PLATFORM_ID STREQUAL "Linux")
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_WINDOWS=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_LINUX=1)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_MACOS=0)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND PLATFORM_ID STREQUAL "Linux")
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_WINDOWS=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_LINUX=1)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_MACOS=0)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND PLATFORM_ID STREQUAL "Darwin")
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_WINDOWS=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_LINUX=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_MACOS=1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND PLATFORM_ID STREQUAL "Darwin")
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_WINDOWS=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_LINUX=0)
    target_compile_definitions(Runtime PUBLIC OKSIJEN_PLATFORM_MACOS=1)
else()
	pMessage(FATAL_ERROR "[CMAKE ERROR] -- UNSUPPORTED PLATFORM. PLEASE CHECK YOUR COMPILER AND PLATFORM.")
endif()
# ------------------------------------------------------------

# -------------------- MACRO DEFINITIONS ---------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(Runtime PUBLIC RUNTIME_API=__declspec\(dllexport\))
    target_compile_definitions(Runtime PUBLIC FORCEINLINE=__forceinline)
    target_compile_definitions(Runtime PUBLIC NOINLINE=__declspec\(noinline\))
    target_compile_definitions(Runtime PUBLIC INLINE=__inline)
    target_compile_definitions(Runtime PUBLIC NULLPTR=decltype\(nullptr\)\(\))
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_definitions(Runtime PUBLIC RUNTIME_API=__attribute__\(\(visibility\(\"default\"\)\)\))
    target_compile_definitions(Runtime PUBLIC FORCEINLINE=__attribute__\(\(always_inline\)\))
    target_compile_definitions(Runtime PUBLIC NOINLINE=__attribute__\(\(noinline\)\))
    target_compile_definitions(Runtime PUBLIC INLINE=__attribute__\(\(always_inline\)\))
    target_compile_definitions(Runtime PUBLIC NULLPTR=nullptr)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(Runtime PUBLIC RUNTIME_API=__attribute__\(\(visibility\(\"default\"\)\)\))
    target_compile_definitions(Runtime PUBLIC FORCEINLINE=__attribute__\(\(always_inline\)\))
    target_compile_definitions(Runtime PUBLIC NOINLINE=__attribute__\(\(noinline\)\))
    target_compile_definitions(Runtime PUBLIC INLINE=__attribute__\(\(always_inline\)\))
    target_compile_definitions(Runtime PUBLIC NULLPTR=nullptr)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GCC")
	target_compile_definitions(Runtime PUBLIC RUNTIME_API=__attribute__\(\(visibility\(\"default\"\)\)\))
	target_compile_definitions(Runtime PUBLIC FORCEINLINE=__attribute__\(\(always_inline\)\))
	target_compile_definitions(Runtime PUBLIC NOINLINE=__attribute__\(\(noinline\)\))
	target_compile_definitions(Runtime PUBLIC INLINE=__attribute__\(\(always_inline\)\))
	target_compile_definitions(Runtime PUBLIC NULLPTR=nullptr)
else()
	pMessage(FATAL_ERROR "[CMAKE ERROR] -- UNSUPPORTED COMPILER. PLEASE CHECK YOUR COMPILER AND PLATFORM.")
endif()
# ------------------------------------------------------------



