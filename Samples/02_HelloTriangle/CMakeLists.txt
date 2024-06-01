cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/02_HelloTriangle/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/02_HelloTriangle/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/02_HelloTriangle" FILES ${SUBDIRECTORIES})
add_executable(HelloTriangle main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(HelloTriangle PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(HelloTriangle PRIVATE Runtime)
# ------------------------------------------------------------