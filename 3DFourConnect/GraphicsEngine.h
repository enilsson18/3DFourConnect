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
#include "Model.h"
#include "Mesh.h"
#include "Text.h"

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
	Shader testCubeShader;

	const unsigned int *SCR_WIDTH;
	const unsigned int *SCR_HEIGHT;

	//list of active models to draw
	std::vector<Model> scene;
	std::vector<Model**> scenePointers;

	//mouse modes
	MouseControlState mouseMode;

	bool clampMouse;
	bool pastClampMouse;

	//temp testing vars
	unsigned int VAO;
	unsigned int VBO;

	//text stuff
	Text textManager;

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

		//Blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//mouse normal callback 
		if (!customCallback) {
			//mouse control State default
			setMouseMode(MouseControlState::POV);

			clampMousePointer = &clampMouse;
			mouseModePointer = &mouseMode;
		}
		else {
			setMouseMode(MouseControlState::CUSTOM);
		}

		//text setup
		textManager = Text(*SCR_WIDTH, *SCR_HEIGHT);

		//Camera
		camera = Camera(*SCR_WIDTH, *SCR_HEIGHT, glm::vec3(0.0, 7*1.5, 20), true);

		cameraPointer = &camera;

		generateTestCube();

		testCubeShader = Shader("resources/shaders/cube.vs", "resources/shaders/cube.fs");
		shader = Shader("resources/shaders/basic_model.vs", "resources/shaders/basic_model.fs");
	}

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
		else {
			mouseMode = state;
		}
	}

	//model functions
	Model &getModel(int index) {
		return scene[index];
	}

	void addModel(Model* &ptr, string const &path) {
		std::cout << "Added model at location: " << path << std::endl;
		scene.push_back(Model(path));
		ptr = &scene[scene.size() - 1];
		scenePointers.push_back(&ptr);

		resetModelPointers();

		//return scene[scene.size() - 1];
	}

	void addModel(Model* &ptr, string const &path, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		std::cout << "Added model at location: " << path << std::endl;
		scene.push_back(Model(path, position, rotation, scale));
		ptr = &scene[scene.size() - 1];
		scenePointers.push_back(&ptr);

		resetModelPointers();

		//return scene[scene.size() - 1];
	}

	void resetModelPointers() {
		for (int i = 0; i < scene.size(); i++) {
			(*scenePointers[i]) = &scene[i];
		}
	}

	void removeModel(Model *model) {
		for (int i = 0; i < scene.size(); i++) {
			if (&scene[i] == model) {
				scene.erase(scene.begin() + i);
				break;
			}
		}
	}

	void addText(std::string text, float x, float y, float scale, glm::vec3 color) {
		textManager.addText(text, x, y, scale, color);
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
		//testing stuff
		
		//generateTestCube();
		for (int i = 0; i < 10; i++) {
			//drawTestCube();
		}
		

		//draw models
		for (int i = 0; i < scene.size(); i++) {
			//drawTestCube(scene[i].position);

			shader.use();

			glm::mat4 projection = camera.projection;
			glm::mat4 view = camera.update();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, scene[i].position);
			model = glm::rotate(model, glm::radians(scene[i].rotation.x), glm::vec3(1.0, 0.0, 0.0));
			model = glm::rotate(model, glm::radians(scene[i].rotation.y), glm::vec3(0.0, 1.0, 0.0));
			model = glm::rotate(model, glm::radians(scene[i].rotation.z), glm::vec3(0.0, 0.0, 1.0));
			model = glm::scale(model, scene[i].scale);	// it's a bit too big for our scene, so scale it down
			shader.setMat4("model", model);
			scene[i].Draw(shader, camera);
		}

		//render text elements
		textManager.render();

		glfwSwapBuffers(window);
		glfwPollEvents();

		return 1;
	}

private:
	//end opengl and free allocated resources
	void terminate() {
		glfwTerminate();
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

	void drawTestCube(glm::vec3 pos) {
		//draw it
		//use shader and set view and projection panes
		testCubeShader.use();
		testCubeShader.setMat4("projection", camera.projection);
		testCubeShader.setMat4("view", camera.update());

		//set coords and scale
		glm::mat4 cubeModel(1);
		cubeModel = glm::translate(cubeModel, pos);
		cubeModel = glm::scale(cubeModel, glm::vec3(1));
		testCubeShader.setMat4("model", cubeModel);

		//bind, draw, then reset bind to vao
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	//NONE CUSTOM INPUT CONTROL SECTION

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