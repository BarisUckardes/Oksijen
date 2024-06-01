cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/03_TexturedQuad/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/03_TexturedQuad/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/03_TexturedQuad" FILES ${SUBDIRECTORIES})
add_executable(TexturedQuad main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(TexturedQuad PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(TexturedQuad PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(TexturedQuad PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")