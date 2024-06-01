cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/01_RedBlueAlternating/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/01_RedBlueAlternating/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/01_RedBlueAlternating" FILES ${SUBDIRECTORIES})
add_executable(RedBlueAlternating main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(RedBlueAlternating PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(RedBlueAlternating PRIVATE Runtime)
# ------------------------------------------------------------