file(GLOB srcs "*.hpp" "*.cpp")

add_executable("Allocation" "${srcs}")

target_link_libraries("Allocation" PUBLIC "ObjectPool")

set_target_properties("Allocation" PROPERTIES FOLDER "Testing")