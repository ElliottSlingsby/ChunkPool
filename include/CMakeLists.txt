add_library("ChunkPool" INTERFACE)

target_include_directories("ChunkPool" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB src "*.hpp" "*.cpp")

add_library("ChunkPool_static" STATIC "${src}")
set_target_properties("ChunkPool_static" PROPERTIES LINKER_LANGUAGE CXX)