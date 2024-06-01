cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/05_NaiveInstancing/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/05_NaiveInstancing/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/05_NaiveInstancing" FILES ${SUBDIRECTORIES})
add_executable(NaiveInstancing main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(NaiveInstancing PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(NaiveInstancing PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(NaiveInstancing PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")