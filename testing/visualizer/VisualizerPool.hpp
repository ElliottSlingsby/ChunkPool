#pragma once

#include <ObjectPool.hpp>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>

#ifndef VISUALIZER_HEIGHT
#define VISUALIZER_HEIGHT 64
#endif

#ifndef VISUALIZER_WIDTH
#define VISUALIZER_WIDTH 1280
#endif

class VisualizerPool : public ObjectPool{
	GLFWwindow* _window = nullptr;

	GLuint _vertShaderId;
	GLuint _geomShaderId;
	GLuint _fragShaderId;

	GLuint _programId;

	GLuint _vertArrayId;
	GLuint _vertBufferId;

	glm::mat4 _matrix;
	std::vector<glm::uvec2> _buffer;

public:
	VisualizerPool(uint64_t chunkSize);
	~VisualizerPool();

	void draw();

	int shouldClose();
};