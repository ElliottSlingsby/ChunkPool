if(OBJECTPOOL_TESTING)
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
endif(OBJECTPOOL_TESTING)