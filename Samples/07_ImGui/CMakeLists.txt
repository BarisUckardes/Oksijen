cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/07_ImGui/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/07_ImGui/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/07_ImGui" FILES ${SUBDIRECTORIES})
add_executable(07_ImGui main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(07_ImGui PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(07_ImGui PRIVATE Runtime)
# ------------------------------------------------------------