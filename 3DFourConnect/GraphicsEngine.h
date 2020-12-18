#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <img/stb_image.h>
//idk but manually importing fixes the problem
//#include <img/ImageLoader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.h>

#include <iostream>

//graphics tools
#include "Camera.h"

//prototypes
//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void window_focus_callback(GLFWwindow* window, int focused);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

//tools
//mouse state, POV for point of view camera and controls, MOUSE for normal mouse movement detection and no camera effect.
enum MouseControlState { POV, MOUSE, CUSTOM };

//Pointer tools
//Window Clamp Mouse
bool *clampMousePointer;
MouseControlState *mouseModePointer;

Camera *cameraPointer;

class GraphicsEngine {
public:
	//normal vars
	GLFWwindow* window;

	Camera camera;

	Shader shader;

	const unsigned int *SCR_WIDTH;
	const unsigned int *SCR_HEIGHT;

	//list of active assets to draw


	//mouse modes
	MouseControlState mouseMode;

	bool clampMouse;
	bool pastClampMouse;

	//temp testing vars
	unsigned int VAO;
	unsigned int VBO;


	//takes in window display name, screen width, screen height, The last is if you want to have custom or preset controll callbacks.
	GraphicsEngine(const char* windowName, const unsigned int *scr_WIDTH, const unsigned int *scr_HEIGHT, bool customCallback) {
		//window setup
		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		SCR_WIDTH = scr_WIDTH;
		SCR_HEIGHT = scr_HEIGHT;

		// glfw window creation
		window = glfwCreateWindow(*scr_WIDTH, *scr_HEIGHT, windowName, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);
		//make callbacks
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		//if the callbacks are not custom
		if (!customCallback) {
			glfwSetCursorPosCallback(window, mouse_callback);
			glfwSetWindowFocusCallback(window, window_focus_callback);
			glfwSetMouseButtonCallback(window, mouse_button_callback);
		}

		// glad: load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}

		// configure global opengl state
		glEnable(GL_DEPTH_TEST);
		stbi_set_flip_vertically_on_load(true);

		//mouse normal callback 
		if (!customCallback) {
			//mouse control State default
			setMouseMode(MouseControlState::POV);

			clampMousePointer = &clampMouse;
			mouseModePointer = &mouseMode;
		}

		//Camera
		camera = Camera(*SCR_WIDTH, *SCR_HEIGHT, glm::vec3(0.0, 0.0, -10), true);

		cameraPointer = &camera;

		generateTestCube();
	}

	void generateTestCube() {
		float cube[] = {
			//back
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			//front
			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,

			//left
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			//right
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,

			 //bottom
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,

			//top
			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
		};

		//init shaders
		shader = Shader("resources/shaders/cube.vs", "resources/shaders/cube.fs");

		//load vbo and make vao
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void drawTestCube() {
		//draw it
		//use shader and set view and projection panes
		shader.use();
		shader.setMat4("projection", camera.projection);
		shader.setMat4("view", camera.update());

		//set coords and scale
		glm::mat4 cubeModel(1);
		cubeModel = glm::translate(cubeModel, glm::vec3(0));
		cubeModel = glm::scale(cubeModel, glm::vec3(1));
		shader.setMat4("model", cubeModel);

		//bind, draw, then reset bind to vao
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	int renderFrame() {
		if (glfwWindowShouldClose(window)) {
			return 0;
		}

		//process input
		//optional camera input
		if (mouseMode != MouseControlState::CUSTOM) {
			processEscapeInput();
			camera.processInput(window);
		}

		//bind frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, *SCR_WIDTH, *SCR_HEIGHT);

		//clear the screen and start next frame
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw
		//generateTestCube();
		for (int i = 0; i < 10; i++) {
			drawTestCube();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		return 1;
	}

	//end opengl and free allocated resources
	void terminate() {
		glfwTerminate();
	}

	//NONE CUSTOM INPUT CONTROL SECTION
	//switch Mouse Modes
	void setMouseMode(MouseControlState state) {
		if (state == MouseControlState::POV) {
			mouseMode = state;
			clampMouse = true;
		}
		else if (state == MouseControlState::MOUSE) {
			clampMouse = false;
			mouseMode = state;
		}
	}

	//releases clamp mouse if locked into the screen.
	void processEscapeInput() {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (clampMouse) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				clampMouse = false;
			}
			else if (pastClampMouse == false) {
				glfwSetWindowShouldClose(window, true);
			}
		}
		else {
			pastClampMouse = clampMouse;
		}
	}
};

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	std::cout << "Failed to create GLFW window" << std::endl;
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on some displays
	glViewport(0, 0, width, height);
}

//focus callback
void window_focus_callback(GLFWwindow* window, int focused) {
	if (*mouseModePointer == MouseControlState::POV) {
		if (focused) {
			*clampMousePointer = true;
		}
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

//NONE CUSTOM MOUSE CALLBACKS
//clicking
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (*mouseModePointer == MouseControlState::POV) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			*clampMousePointer = true;
		}
	}
}

//mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (*mouseModePointer == MouseControlState::POV) {
		if (*clampMousePointer) {
			(*cameraPointer).mouseInputPOV(window, xpos, ypos);
		}
	}
}



#endif