if(CHUNKPOOL_VISUALIZER)
	#GLFW
	option(GLFW_BUILD_DOCS "" OFF)
	option(GLFW_BUILD_EXAMPLES "" OFF)
	option(GLFW_BUILD_TESTS "" OFF)
	option(GLFW_INSTALL "" OFF)

	add_external_tar("glfw-3.2.1" "https://github.com/glfw/glfw/archive/3.2.1.tar.gz" "")

	set_target_properties("glfw" PROPERTIES FOLDER "Thirdparty")

	# GLAD
	add_subdirectory("glad-gl-4.5")

	set_target_properties("glad" PROPERTIES FOLDER "Thirdparty")

	# GLM
	option(GLM_STATIC_LIBRARY_ENABLE "" ON)

	add_external_tar("glm-0.9.8.3" "https://github.com/g-truc/glm/archive/0.9.8.3.tar.gz" "")

	set_target_properties("glm_static" PROPERTIES FOLDER "Thirdparty")
endif(CHUNKPOOL_VISUALIZER)
	
if(CHUNKPOOL_UNITTEST)
	# GTest
	option(gtest_force_shared_crt "" ON)
	
	add_external_tar("googletest-release-1.8.0" "https://github.com/google/googletest/archive/release-1.8.0.tar.gz" "")
	
	set_target_properties("gmock" PROPERTIES FOLDER "Thirdparty")
	set_target_properties("gmock_main" PROPERTIES FOLDER "Thirdparty")
	set_target_properties("gtest" PROPERTIES FOLDER "Thirdparty")
	set_target_properties("gtest_main" PROPERTIES FOLDER "Thirdparty")
endif(CHUNKPOOL_UNITTEST)