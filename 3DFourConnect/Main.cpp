//utility
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

#include <FileSystem>
#include <iostream>

#include <vector>
#include <chrono>
#include <cmath>
#include <ctime>
#include <windows.h>

//written class refrences
#include "GraphicsEngine.h"
#include "Text.h"

//Board and game classes
#include "GameManager.h"
#include "Board.h"
#include "Piece.h"

//file paths
const char* RedModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\redball\\redball.obj";
const char* BlueModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\blueball\\blueball.obj";
const char* OutlineModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\outlineball\\outlineball.obj";
const char* BoardModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\3dfourconnect\\3dfourconnectFIXED.obj";
const char* BackpackModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\testing\\backpack\\backpack.obj";

//skybox paths
std::vector<const char*> cloudySkybox
{
	"resources/textures/CloudySkyBox/cubemap_1.jpg",
	"resources/textures/CloudySkyBox/cubemap_3.jpg",
	"resources/textures/CloudySkyBox/cubemap_4.jpg",
	"resources/textures/CloudySkyBox/cubemap_5.jpg",
	"resources/textures/CloudySkyBox/cubemap_0.jpg",
	"resources/textures/CloudySkyBox/cubemap_2.jpg",
};

std::vector<const char*> galexySkybox
{
	"resources/textures/SpaceSkyBox/rightImage.png",
	"resources/textures/SpaceSkyBox/leftImage.png",
	"resources/textures/SpaceSkyBox/upImage.png",
	"resources/textures/SpaceSkyBox/downImage.png",
	"resources/textures/SpaceSkyBox/frontImage.png",
	"resources/textures/SpaceSkyBox/backImage.png",
};

//prototypes
//control callback for clicking the mouse
void mouse_button_callback_custom(GLFWwindow* window, int button, int action, int mods);
//control callback for moving the mouse
void mouse_callback_custom(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

bool fpsCounter = true;
const double fps = 60;

GameManager *gM;

int main() {
	//make the graphics engine (Jordan: Do not focus too much on this, it is very complicated and not relevant to the problem.
	GraphicsEngine graphics("3D Four Connect", &SCR_WIDTH, &SCR_HEIGHT, true);

	//add all the models that are going to be used immediatley
	graphics.addModel(BoardModelPath);
	graphics.addModel(RedModelPath);
	graphics.addModel(BlueModelPath);
	graphics.addModel(OutlineModelPath);

	//set skybox
	graphics.setSkybox(cloudySkybox);

	//make the board and game manager
	GameManager gameManager(graphics, graphics.camera, glm::vec3(0, 0, 0));

	//set pointers
	gM = &gameManager;

	//set camera starting pos
	graphics.camera.setPos(glm::vec3(0.0f, 7 * 1.5f, 40.0f));

	//set callbacks
	glfwSetCursorPosCallback(graphics.window, mouse_callback_custom);
	glfwSetMouseButtonCallback(graphics.window, mouse_button_callback_custom);

	//add text
	//graphics.textManager.addText("This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

	//add test piece
	gameManager.board.addPiece(Piece::Color::BLUE, 0, 3, 0);
	gameManager.board.addPiece(Piece::Color::RED, 1, 3, 0);

	int fpsCount = 0;
	int fpsCounter = 0;

	int gameState = 1;
	while (gameState == 1) {
		//start timer
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

		//input

		//update the board and game

		gameManager.update();

		//std::cout << graphics.camera.yaw << " " << graphics.camera.pitch << std::endl;

		//render frame
		gameState = graphics.renderFrame();

		//end of timer sleep and normalize the clock
		std::chrono::system_clock::time_point after = std::chrono::system_clock::now();
		std::chrono::microseconds difference(std::chrono::time_point_cast<std::chrono::microseconds>(after) - std::chrono::time_point_cast<std::chrono::microseconds>(now));

		//count the fps
		int diffCount = difference.count();
		if (diffCount == 0) {
			diffCount = 1;
		}

		int sleepDuration = ((1000000 / fps * 1000) - diffCount) / 1000000;

		//output fps
		fpsCount += 1;
		fpsCounter += 1000000 / diffCount;

		if (fpsCount % int(fps) == 0) {
			if (fpsCounter) {
				std::cout << "\rFPS: " << fpsCounter / fpsCount;
			}
			fpsCount = 0;
			fpsCounter = 0;
		}

		if (sleepDuration < 0) {
			sleepDuration = 0;
		}

		//std::cout << sleepDuration << std::endl;
		Sleep(sleepDuration);
	}

	return 0;
}

//clicking
void mouse_button_callback_custom(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		(*gM).leftClick();
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		(*gM).rightClick();
	}
}

//mouse movement
void mouse_callback_custom(GLFWwindow* window, double xpos, double ypos)
{
	(*gM).mousePieceSelect(xpos, ypos);
}