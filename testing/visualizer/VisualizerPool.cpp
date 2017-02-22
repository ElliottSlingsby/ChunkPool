#include "VisualizerPool.hpp"

#include <iostream>

const char vertShaderSrc[] =
	"#version 450\n"

	"in vec2 position;"

	"void main(){"
		"gl_Position = vec4(position, 0.0, 1.0);"
	"};";

const char geomShaderSrc[] =
	"#version 450\n"

	"uniform mat4 matrix;"
	"uniform vec2 resolution;"

	"uniform uint size;"
	"uniform uint height;"

	"layout (points) in;"
	"layout (triangle_strip, max_vertices = 4) out;"

	"void main(){"
		"vec2 location = vec2((gl_in[0].gl_Position.x / size) * resolution.x, (gl_in[0].gl_Position.y / size) * resolution.x);"

		"gl_Position = matrix * vec4(location.x, 0.0, 0.0, 1.0);"
		"EmitVertex();"

		"gl_Position = matrix * vec4(location.x, height, 0.0, 1.0);"
		"EmitVertex();"

		"gl_Position = matrix * vec4(location.y, 0.0, 0.0, 1.0);"
		"EmitVertex();"

		"gl_Position = matrix * vec4(location.y, height, 0.0, 1.0);"
		"EmitVertex();"
	"};";

const char fragShaderSrc[] =
	"#version 450\n"

	"void main(){"
		"gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
	"};";

void glfwError(int error, const char* info){
	std::cerr << "GLFW error - " << error << " : " << info << std::endl;
}

void keyAction(GLFWwindow* _window, int key, int scancode, int action, int mod){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

int compileShader(GLenum type, GLuint* id, const char* src){
	*id = glCreateShader(type);
	glShaderSource(*id, 1, &src, nullptr);
	glCompileShader(*id);

	GLint success = GL_FALSE;
	glGetShaderiv(*id, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE){
		GLint length = 0;
		glGetShaderiv(*id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*)std::malloc(length);
		glGetShaderInfoLog(*id, length, &length, &message[0]);

		std::cerr << "GLSL error - " << message << std::endl;

		std::free(message);

		return false;
	}

	return true;
}

VisualizerPool::VisualizerPool() : ChunkPool(0){
	// Create _window
	if (!glfwInit())
		return;

	glfwSetErrorCallback(glfwError);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	_window = glfwCreateWindow(VISUALIZER_WIDTH, VISUALIZER_HEIGHT, "Visualizer", nullptr, nullptr);

	if (!_window){
		glfwTerminate();
		return;
	}

	glfwSetKeyCallback(_window, keyAction);

	glfwMakeContextCurrent(_window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	// Compile shaders
	bool compiled = true;

	if (!compileShader(GL_VERTEX_SHADER, &_vertShaderId, vertShaderSrc))
		compiled = false;

	if (!compileShader(GL_GEOMETRY_SHADER, &_geomShaderId, geomShaderSrc))
		compiled = false;

	if (!compileShader(GL_FRAGMENT_SHADER, &_fragShaderId, fragShaderSrc))
		compiled = false;

	if (!compiled){
		glfwDestroyWindow(_window);
		glfwTerminate();

		glDeleteShader(_vertShaderId);
		glDeleteShader(_geomShaderId);
		glDeleteShader(_fragShaderId);

		return;
	}

	// Create shader program
	_programId = glCreateProgram();

	glAttachShader(_programId, _vertShaderId);
	glAttachShader(_programId, _geomShaderId);
	glAttachShader(_programId, _fragShaderId);

	glLinkProgram(_programId);

	GLuint positionLoc = glGetAttribLocation(_programId, "position");

	// Create vertex array
	glGenVertexArrays(1, &_vertArrayId);
	glBindVertexArray(_vertArrayId);

	glEnableVertexAttribArray(positionLoc);

	// Set shader attribs
	glGenBuffers(1, &_vertBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertBufferId);

	glVertexAttribPointer(positionLoc, 2, GL_UNSIGNED_INT, GL_FALSE, sizeof(glm::uvec2), (void*)0);
}

VisualizerPool::~VisualizerPool(){
	glDeleteShader(_vertShaderId);
	glDeleteShader(_fragShaderId);

	glDeleteProgram(_programId);

	glfwDestroyWindow(_window);
	glfwTerminate();
}

void VisualizerPool::draw(){
	int width, height;
	glfwGetFramebufferSize(_window, &width, &height);

	if (!width)
		width = 1;
	if (!height)
		height = 1;

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_programId);

	GLuint matrixLoc = glGetUniformLocation(_programId, "matrix");
	GLuint resolutionLoc = glGetUniformLocation(_programId, "resolution");
	GLuint sizeLoc = glGetUniformLocation(_programId, "size");
	GLuint heightLoc = glGetUniformLocation(_programId, "height");

	_matrix = glm::ortho(0.f, (float)width, (float)height, 0.f, 1.f, -1.f);
	glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, (const GLfloat*)&_matrix[0]);

	glUniform2f(resolutionLoc, (float)width, (float)height);

	glUniform1ui(heightLoc, height);

	// DESPERATELY NEEDS UPDATING!

	//_buffer.clear();
	//_buffer.reserve(_nodeObjects.size());
    
	//for (const MemoryNode& node : _nodeObjects){
	//	if (node.active)
	//		_buffer.push_back({ node.location, end(node) });
	//}

	//glUniform1ui(sizeLoc, (GLuint)_mainMemorySize);

	//glNamedBufferData(_vertBufferId, _buffer.size() * sizeof(glm::uvec2), _buffer.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(_vertArrayId);
	glDrawArrays(GL_POINTS, 0, (GLsizei)(_buffer.size()));

	glfwSwapBuffers(_window);
	glfwPollEvents();
}

int VisualizerPool::shouldClose(){
	if (!_window)
		return 1;

	return glfwWindowShouldClose(_window);
}
