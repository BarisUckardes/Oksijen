cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/04_RotatingCube/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/03_RotatingCube/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/04_RotatingCube" FILES ${SUBDIRECTORIES})
add_executable(RotatingCube main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(RotatingCube PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(RotatingCube PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(RotatingCube PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")