#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.h>

#include <iostream>

#include "Text.h"

//graphics tools
#include "GraphicsEngine.h"
#include "Camera.h"
#include "Model.h"
#include "Board.h"
#include "Piece.h"

class GameManager {
public:
	GraphicsEngine *graphics;
	Camera *camera;

	Board board;

	Piece::Color currentTurn;

	glm::vec3 mouseRay;

	Piece testPiece;

	GameManager(GraphicsEngine &graphics, Camera &camera, glm::vec3 pos) {
		this->graphics = &graphics;
		this->camera = &camera;

		board = Board(graphics, pos);

		currentTurn = Piece::Color::RED;

		testPiece = Piece(&graphics, Piece::RED, glm::vec3(0));
	}

	bool update() {
		(*camera).processInput((*graphics).window);
		(*camera).lookAtTarget(board.getCenter());
		(*testPiece.model).setPosition((*camera).pos + mouseRay * 1.0f);

		return true;
	}

	void checkGameInput() {

	}

	void leftClick() {

	}

	void rightClick() {

	}

	//take the camera 
	void mousePieceSelect(double x, double y) {
		//calculate the vector displacement of the cursor on the screen from the cameras perspective
		float nearPlaneWidth = (tan((16.0f / 9.0f) * (*camera).fov * (3.1415926535 / 180)) / (*camera).nearPlane) * 2;
		float nearPlaneHeight = (tan((*camera).fov * (3.1415926535 / 180)) / (*camera).nearPlane) * 2;

		glm::vec3 dist = 20*(*camera).nearPlane * (*camera).Front;
		glm::vec3 xOffset = (*camera).Right * float(x * (nearPlaneWidth / *(*graphics).SCR_WIDTH) - (nearPlaneWidth/2));
		glm::vec3 yOffset = -(*camera).Up * float(y * (nearPlaneHeight / *(*graphics).SCR_HEIGHT) - (nearPlaneHeight/2));

		mouseRay = (dist + xOffset + yOffset) * glm::vec3(1,1,1);

		//std::cout << float(x / *(*graphics).SCR_WIDTH) << std::endl;
		//printVector((*camera).pos + mouseRay * 20.0f);
		printVector(mouseRay);
		//printVector((*graphics).getModel(0).position - (*camera).pos);
	}

	//utility
	void printVector(glm::vec3 vec) {
		std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
	}
};

#endif