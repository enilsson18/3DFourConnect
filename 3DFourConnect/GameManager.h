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

	float camFollowDistance;

	Piece::Color currentTurn;
	glm::vec3 mouseRay;

	enum Stage { TESTING, SETUP, PLAY, END };
	Stage stage;

	Piece previewPiece;

	//testing
	Piece testPiece;

	GameManager(GraphicsEngine &graphics, Camera &camera, glm::vec3 pos) {
		this->graphics = &graphics;
		this->camera = &camera;

		//default values init
		camFollowDistance = 40.0f;

		//construct the game setup
		board = Board(graphics, pos);

		//board.addPiece(Piece::Color::BLUE, 0, 3, 0);

		currentTurn = Piece::Color::RED;

		stage = Stage::PLAY;

		//testing setup
		if (stage == Stage::TESTING) {
			testPiece = Piece(&graphics, Piece::RED, glm::vec3(0));
		}
	}

	void update() {
		std::cout << (*graphics).scene.size() << std::endl;

		//main game info
		if (stage == Stage::PLAY) {
			//if the preview piece is not set then make it
			if (previewPiece.type == Piece::NONE) {
				previewPiece = Piece(graphics, currentTurn, glm::vec3(0));
			}

			//check key input
			checkGameInput();

			//make cam look at target and stay locked at a set distance
			(*camera).setPos(board.getCenter() + glm::normalize((*camera).pos - board.getCenter()) * camFollowDistance);
			(*camera).lookAtTarget(board.getCenter());

			//update vectors
			(*camera).updateCameraVectors();

			//bind and rotate preview piece
			bindPreviewPiece();
		}

		if (stage == Stage::TESTING) {
			(*camera).processInput((*graphics).window);
			(*camera).lookAtTarget(board.getCenter());
			(*testPiece.model).setPosition((*camera).pos + mouseRay * 1.0f);
		}
	}

	void checkGameInput() {
		float accel = 0.02;
		(*camera).deceleration = 0.01;

		//camera controls
		//up
		if (glfwGetKey((*graphics).window, GLFW_KEY_UP) | glfwGetKey((*graphics).window, GLFW_KEY_W) == GLFW_PRESS) {
			(*camera).vel += accel * (*camera).Up;
		}
		//down
		if (glfwGetKey((*graphics).window, GLFW_KEY_DOWN) | glfwGetKey((*graphics).window, GLFW_KEY_S) == GLFW_PRESS) {
			(*camera).vel -= accel * (*camera).Up;
		}
		//right
		if (glfwGetKey((*graphics).window, GLFW_KEY_RIGHT) | glfwGetKey((*graphics).window, GLFW_KEY_D) == GLFW_PRESS) {
			(*camera).vel += accel * (*camera).Right;
		}
		//left
		if (glfwGetKey((*graphics).window, GLFW_KEY_LEFT) | glfwGetKey((*graphics).window, GLFW_KEY_A) == GLFW_PRESS) {
			(*camera).vel -= accel * (*camera).Right;
		}

		//update physics
		(*camera).updatePhysics();
	}

	//check if somebody won
	Piece::Color checkWin() {
		//HORIZONTAL OR VERTICAL
		//PARALLEL TO X-AXIS
		//RED
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 4; z++) {
				for (int x = 0; x < 4; x++) {
					if (board.data[x][y][z].type != Piece::RED) {
						break;
					}
					else if (x == 3) {
						return Piece::Color::RED;
					}
				}
			}
		}
		//BLUE
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 4; z++) {
				for (int x = 0; x < 4; x++) {
					if (board.data[x][y][z].type != Piece::BLUE) {
						break;
					}
					else if (x == 3) {
						return Piece::Color::BLUE;
					}
				}
			}
		}

		//PARALLEL TO Y-AXIS
		//RED
		for (int x = 0; x < 4; x++) {
			for (int z = 0; z < 4; z++) {
				for (int y = 0; y < 4; y++) {
					if (board.data[x][y][z].type != Piece::RED) {
						break;
					}
					else if (y == 3) {
						return Piece::Color::RED;
					}
				}
			}
		}
		//BLUE
		for (int x = 0; x < 4; x++) {
			for (int z = 0; z < 4; z++) {
				for (int y = 0; y < 4; y++) {
					if (board.data[x][y][z].type != Piece::BLUE) {
						break;
					}
					else if (y == 3) {
						return Piece::Color::BLUE;
					}
				}
			}
		}

		//PARALLEL TO Z-AXIS
		//RED
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				for (int z = 0; z < 4; z++) {
					if (board.data[x][y][z].type != Piece::RED) {
						break;
					}
					else if (z == 3) {
						return Piece::Color::RED;
					}
				}
			}
		}
		//BLUE
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				for (int z = 0; z < 4; z++) {
					if (board.data[x][y][z].type != Piece::BLUE) {
						break;
					}
					else if (z == 3) {
						return Piece::Color::BLUE;
					}
				}
			}
		}

		//DIAGONAL
		//X-FACE
		//RED
		for (int z = 0; z < 4; z++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][i][z].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		for (int z = 0; z < 4; z++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][3 - i][z].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		//BLUE
		for (int z = 0; z < 4; z++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][i][z].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}
		for (int z = 0; z < 4; z++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][3 - i][z].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}

		//Y-FACE
		//RED
		for (int y = 0; y < 4; y++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][y][i].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		for (int y = 0; y < 4; y++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[3 - i][y][i].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		//BLUE
		for (int y = 0; y < 4; y++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[i][y][i].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}
		for (int y = 0; y < 4; y++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[3 - i][y][i].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}

		//Z-FACE
		//RED
		for (int x = 0; x < 4; x++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[x][i][i].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		for (int x = 0; x < 4; x++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[x][3-i][i].type != Piece::RED) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::RED;
				}
			}
		}
		//BLUE
		for (int x = 0; x < 4; x++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[x][i][i].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}
		for (int x = 0; x < 4; x++) {
			for (int i = 0; i < 4; i++) {
				if (board.data[x][3 - i][i].type != Piece::BLUE) {
					break;
				}
				else if (i == 3) {
					return Piece::Color::BLUE;
				}
			}
		}

		//DIAGONAL X DIAGONAL
		//RED
		for (int i = 0; i < 4; i++) {
			if (board.data[i][i][i].type != Piece::RED) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::RED;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[3-i][i][i].type != Piece::RED) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::RED;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[i][3-i][i].type != Piece::RED) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::RED;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[i][i][3-i].type != Piece::RED) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::RED;
			}
		}
		//BLUE
		for (int i = 0; i < 4; i++) {
			if (board.data[i][i][i].type != Piece::BLUE) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::BLUE;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[3 - i][i][i].type != Piece::BLUE) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::BLUE;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[i][3 - i][i].type != Piece::BLUE) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::BLUE;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (board.data[i][i][3 - i].type != Piece::BLUE) {
				break;
			}
			else if (i == 3) {
				return Piece::Color::BLUE;
			}
		}

		return Piece::Color::NONE;
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
		//printVector(mouseRay);
		//printVector((*graphics).getModel(0).position - (*camera).pos);
	}

	void bindPreviewPiece() {
		//update position
		glm::vec3 displacement = glm::vec3(3.0f, 0.0f, 3.0f);

		(*previewPiece.model).setPosition((*camera).pos + displacement * ((*camera).Front + (*camera).Right + (*camera).Up));

		//update rotation
		glm::vec3 rotationDisplacement = glm::vec3(1.0f, 2.0f, 3.0f);
	}

	//utility
	void printVector(glm::vec3 vec) {
		std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
	}
};

#endif