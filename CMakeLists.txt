cmake_minimum_required(VERSION 3.26)

# ------------------ PROJECT NAME ------------------
project("Oksijen")
# --------------------------------------------------

# ------------------ CXX FLAGS ---------------------
if(POLICY CMP0091)
  cmake_policy(SET CMP0141 NEW)
  cmake_policy(SET CMP0077 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
# --------------------------------------------------

# --------------- COMPILER VERSIONS ----------------
    message(STATUS "Platform is Windows, Using MSVC Compiler Version ${MSVC_VERSION}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Platform is Unix, Using Clang Compiler Version ${CMAKE_CXX_COMPILER_VERSION}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Platform is Unix, Using GCC Compiler Version ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(FATAL_ERROR "[CMAKE ERROR] -- UNSUPPORTED COMPILER. PLEASE CHANGE YOUR COMPILER.")
endif ()
# --------------------------------------------------

# ------------------ SUBDIRECTORIES ----------------
add_subdirectory("Source/Runtime")

add_subdirectory("Samples/01_RedBlueAlternating")
add_subdirectory("Samples/02_HelloTriangle")
add_subdirectory("Samples/03_TexturedQuad")
add_subdirectory("Samples/04_RotatingCube")
add_subdirectory("Samples/05_NaiveInstancing")
add_subdirectory("Samples/06_ComputeTexture")
add_subdirectory("Samples/07_ImGui")
add_subdirectory("Samples/08_SimpleAssimpImporter")
add_subdirectory("Samples/09_SimpleLighting")
add_subdirectory("Samples/10_SimplePBR")
add_subdirectory("Samples/11_SponzaImport")
# --------------------------------------------------