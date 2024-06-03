cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/09_SimpleLighting/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/09_SimpleLighting/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/09_SimpleLighting" FILES ${SUBDIRECTORIES})
add_executable(SimpleLighting main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(SimpleLighting PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(SimpleLighting PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(SimpleLighting PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")