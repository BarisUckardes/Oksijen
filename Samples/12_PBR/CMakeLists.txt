cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/12_PBR/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/12_PBR/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/12_PBR" FILES ${SUBDIRECTORIES})
add_executable(PBR main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(PBR PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(PBR PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(PBR PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")