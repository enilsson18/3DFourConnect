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

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const double fps = 60;

int main() {
	GraphicsEngine graphics("3D Four Connect", &SCR_WIDTH, &SCR_HEIGHT, true);

	int fpsCount = 0;
	int fpsCounter = 0;

	int gameState = 1;
	while (gameState == 1) {
		//start timer
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

		//input

		//update game world info


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
			std::cout << "\rFPS: " << fpsCounter / fpsCount;
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