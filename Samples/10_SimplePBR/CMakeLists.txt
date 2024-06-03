cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/10_SimplePBR/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/10_SimplePBR/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/10_SimplePBR" FILES ${SUBDIRECTORIES})
add_executable(SimplePBR main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(SimplePBR PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(SimplePBR PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(SimplePBR PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")