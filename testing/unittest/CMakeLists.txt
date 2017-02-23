file(GLOB src "*.hpp" "*.cpp")

add_executable("UnitTest" "${src}")

target_link_libraries("UnitTest" PUBLIC "ChunkPool")

target_link_libraries("UnitTest" PUBLIC "gtest")

set_target_properties("UnitTest" PROPERTIES FOLDER "Testing")