cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/14_SpotLight/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/14_SpotLight/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/14_SpotLight" FILES ${SUBDIRECTORIES})
add_executable(SpotLight main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(SpotLight PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(SpotLight PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(SpotLight PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")