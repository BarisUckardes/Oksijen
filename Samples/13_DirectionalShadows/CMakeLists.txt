cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/13_DirectionalShadows/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/13_DirectionalShadows/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/13_DirectionalShadows" FILES ${SUBDIRECTORIES})
add_executable(DirectionalShadows main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(DirectionalShadows PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(DirectionalShadows PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(DirectionalShadows PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")