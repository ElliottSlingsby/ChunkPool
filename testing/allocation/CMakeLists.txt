file(GLOB srcs "*.hpp" "*.cpp")

add_executable("Threading" "${srcs}")

target_link_libraries("Threading" PUBLIC "ObjectPool")

set_target_properties("Threading" PROPERTIES FOLDER "Testing")