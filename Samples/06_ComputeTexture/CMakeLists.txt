cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/06_ComputeTexture/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/06_ComputeTexture/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/06_ComputeTexture" FILES ${SUBDIRECTORIES})
add_executable(ComputeTexture  ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(ComputeTexture PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(ComputeTexture PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(ComputeTexture PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")