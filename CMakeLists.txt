cmake_minimum_required(VERSION 3.26)

# ------------------ PROJECT NAME ------------------
project("Oksijen")
# --------------------------------------------------

# ------------------ CXX FLAGS ---------------------
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
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

# --------------------------------------------------