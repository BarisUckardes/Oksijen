cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/08_SimpleAssimpImporter/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/08_SimpleAssimpImporter/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/08_SimpleAssimpImporter" FILES ${SUBDIRECTORIES})
add_executable(SimpleAssimpImporter main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(SimpleAssimpImporter PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(SimpleAssimpImporter PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(SimpleAssimpImporter PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")