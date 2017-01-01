file(GLOB src "*.hpp" "*.cpp")

add_executable("Visualizer" "${src}")

target_link_libraries("Visualizer" PUBLIC "ObjectPool")

target_link_libraries("Visualizer" PUBLIC "glad")
target_link_libraries("Visualizer" PUBLIC "glfw")
target_link_libraries("Visualizer" PUBLIC "glm")

set_target_properties("Visualizer" PROPERTIES FOLDER "Testing")

if(MSVC)
	set_target_properties("Visualizer" PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	set_target_properties("Visualizer" PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	set_target_properties("Visualizer" PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(MSVC)