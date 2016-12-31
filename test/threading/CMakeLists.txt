file(GLOB srcs "*.hpp" "*.cpp")

add_executable("Allocation" "${srcs}")

target_link_libraries("Allocation" PUBLIC "ObjectPool")