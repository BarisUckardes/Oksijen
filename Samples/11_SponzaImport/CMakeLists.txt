cmake_minimum_required(VERSION 3.26)

# ---------------------- CMAKE INCLUDES ----------------------
include(${CMAKE_SOURCE_DIR}/Scripts/CMake/FetchLibraries.cmake)
# ------------------------------------------------------------

# ---------------------- FILE ADDITION -----------------------
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/Samples/11_SponzaImport/**.cpp" "${CMAKE_SOURCE_DIR}/Samples/11_SponzaImport/**.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/Samples/11_SponzaImport" FILES ${SUBDIRECTORIES})
add_executable(SponzaImport main.cpp ${SUBDIRECTORIES})
# ------------------------------------------------------------

# -------------------- INCLUDE LIBRARIES ---------------------
target_include_directories(SponzaImport PRIVATE "${CMAKE_SOURCE_DIR}/Source")
# ------------------------------------------------------------

# --------------------- LINK LIBRARIES -----------------------
target_link_libraries(SponzaImport PRIVATE Runtime)
# ------------------------------------------------------------

target_compile_definitions(SponzaImport PUBLIC RES_PATH="${CMAKE_SOURCE_DIR}/Resources")