#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.h>

#include <chrono>
#include <thread>
#include <future>
#include <iostream>

#include "Text.h"

//graphics tools
#include "GraphicsEngine.h"
#include "Camera.h"
#include "Model.h"
#include "Board.h"
#include "Piece.h"

//prototypes

//manages the graphics, controls, and game data of 3d Four Connect
class GameManager {
public:
	//callbacks
	void(*winCallback)(Piece::Color) = nullptr;
	void(*placePieceCallback)(Piece::Color, glm::vec3) = nullptr;


	GraphicsEngine *graphics = nullptr;
	Camera *camera = nullptr;

	Board board;

	bool rightClickStatus;
	bool leftClickStatus;
	glm::vec2 mousePos;

	float camFollowDistance;

	Piece::Color currentTurn;
	glm::vec3 mouseRay;

	enum Stage { TESTING, SETUP, PLAY, DATA, END };
	Stage stage;

	Piece previewPiece;
	Piece outlinePiece;

	//testing
	Piece testPiece;

	//neutral game manager that is meant to mainly handle sheer events and status of the board rather than graphics and controls
	//optimal for a server storing data
	GameManager() {
		board = Board();

		currentTurn = Piece::Color::RED;

		stage = Stage::DATA;
	}

	GameManager(GraphicsEngine &graphics, Camera &camera, glm::vec3 pos) {
		this->graphics = &graphics;
		this->camera = &camera;

		//default values init
		camFollowDistance = 40.0f;

		rightClickStatus = false;
		leftClickStatus = false;

		mousePos = glm::vec2(0);

		//construct the game setup
		board = Board(graphics, pos);

		currentTurn = Piece::Color::RED;

		stage = Stage::PLAY;

		//play setup
		if (stage == Stage::PLAY) {
			outlinePiece = Piece(&graphics, Piece::Color::OUTLINE, glm::vec3(0));
			outlinePiece.asset->visible = false;
		}

		//testing setup
		if (stage == Stage::TESTING) {
			testPiece = Piece(&graphics, Piece::RED, glm::vec3(0));
		}
	}

	void update() {
		//std::cout << (*graphics).scene.size() << std::endl;

		//non-graphical data based game
		if (stage == Stage::DATA) {
			//check win case
			Piece::Color win = checkWin();
			if (win != Piece::Color::NONE) {
				string text;

				cout << endl;
				if (win == Piece::Color::BLUE) {
					cout << "BLUE WINS!" << endl;
					text = "Blue Wins!";
				}
				else if (win == Piece::Color::RED) {
					cout << "RED WINS!" << endl;
					text = "Red Wins!";
				}
				//nobody wins
				else if (board.fullBoard()) {
					cout << "NOBODY WINS" << endl;
					text = "Nobody Wins!";
				}

				//graphics->textManager.addText(text, 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
				if (winCallback != nullptr) {
					winCallback(win);
				}

				board.clearBoard();
			}
		}

		//main game info
		if (stage == Stage::PLAY) {
			//if the preview piece is not set then make it
			if (previewPiece.type != currentTurn) {
				graphics->removeAsset(previewPiece.asset);
				previewPiece = Piece(graphics, currentTurn, glm::vec3(0));
			}

			//check key input
			checkGameInput();

			//make cam look at target and stay locked at a set distance
			(*camera).setPos(board.getCenter() + normalize((*camera).pos - board.getCenter()) * camFollowDistance);
			(*camera).lookAtTarget(board.getCenter());

			//update vectors
			(*camera).updateCameraVectors();

			//update mouse ray
			updateMouseRay();

			//find selected pieces
			glm::vec3 selectedPiece = checkSelectPiece();

			//handle selecting pieces
			//if a piece is selected and it has a type NONE
			if (selectedPiece != glm::vec3(-1) && board.data[(int)selectedPiece.x][(int)selectedPiece.y][(int)selectedPiece.z].type == Piece::Color::NONE) {
				//set outline piece location and visibility
				outlinePiece.asset->setPosition(board.getPiecePosFromCoord((int)selectedPiece.x, (int)selectedPiece.y, (int)selectedPiece.z));
				outlinePiece.asset->visible = true;

				//check for right click or left click events to set piece
				if (leftClickStatus) {
					board.addPiece(currentTurn, (int)selectedPiece.x, (int)selectedPiece.y, (int)selectedPiece.z);
					switchTurn();

					//std::cout << "placed piece" << std::endl;
					//callback
					if (placePieceCallback != nullptr) {
						//std::cout << "called callback" << std::endl;
						placePieceCallback(board.data[(int)selectedPiece.x][(int)selectedPiece.y][(int)selectedPiece.z].type, selectedPiece);
					}
				}
			}
			else {
				outlinePiece.asset->visible = false;
			}

			//bind and rotate preview piece
			bindPreviewPiece();

			//check win case
			Piece::Color win = checkWin();
			if (win != Piece::Color::NONE){
				string text;

				cout << endl;
				if (win == Piece::Color::BLUE) {
					cout << "BLUE WINS!" << endl;
					text = "Blue Wins!";
				}
				else if (win == Piece::Color::RED) {
					cout << "RED WINS!" << endl;
					text = "Red Wins!";
				}
				//nobody wins
				else if (board.fullBoard()) {
					cout << "NOBODY WINS" << endl;
					text = "Nobody Wins!";
				}

				if (winCallback != nullptr) {
					//std::cout << "callback call" << std::endl;
					winCallback(win);
				}
				//graphics->textManager.addText(text, 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
				board.clearBoard();
			}
		}

		if (stage == Stage::TESTING) {
			(*camera).processInput((*graphics).window);
			(*camera).lookAtTarget(board.getCenter());
			testPiece.asset->setPosition((*camera).pos + mouseRay * 20.0f);
		}

		//reset mouse buttons
		rightClickStatus = false;
		leftClickStatus = false;
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

	void switchTurn() {
		if (currentTurn == Piece::Color::BLUE) {
			currentTurn = Piece::Color::RED;
		}
		else if (currentTurn == Piece::Color::RED) {
			currentTurn = Piece::Color::BLUE;
		}
	}

	void leftClick() {
		leftClickStatus = true;
	}

	void rightClick() {
		rightClickStatus = true;
	}
	
	//returns int positions of where the pieces are if they are selected
	glm::vec3 checkSelectPiece() {
		float closestLength = -1;
		glm::vec3 closestPos = glm::vec3(-1);
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					//if the piece is not filled already and is being intersected by the line get the distance between the line and the piece
					float length = checkLinePieceIntersection(board.data[x][y][z], mouseRay);

					//if a piece is selected (not -1) then add it if it is either the closest or the first selected piece.
					if (length != -1 && (closestLength == -1 || length < closestLength)) {
						closestLength = length;
						closestPos = glm::vec3(x, y, z);
					}
				}
			}
		}

		return closestPos;
	}

	//find the distance between the piece targeted and the camera. Then multiply this by the normalized vector of the line from the camera position and compare if they are close enough.
	//returns the distance
	float checkLinePieceIntersection(Piece piece, glm::vec3 line) {
		float dist = glm::length(camera->pos - piece.asset->position);
		glm::vec3 linePoint = line * dist + camera->pos;

		float vecLength = glm::length(linePoint - piece.asset->position);
		if (vecLength < piece.colliderRadius) {
			return vecLength;
		}
		return -1;
	}

	//update mouse pos
	void mouseUpdate(double x, double y) {
		mousePos = glm::vec2(x, y);
	}

	//find the vector ray where the mouse is looking
	void updateMouseRay() {
		//calculate the vector displacement of the cursor on the screen from the cameras perspective
		float nearPlaneWidth = (tan((16.0f / 9.0f) * (*camera).fov * (3.1415926535 / 180)) / (*camera).nearPlane) * 2;
		float nearPlaneHeight = (tan((*camera).fov * (3.1415926535 / 180)) / (*camera).nearPlane) * 2;

		glm::vec3 dist = 1 * (*camera).nearPlane * (*camera).Front;
		glm::vec3 xOffset = (*camera).Right * float(mousePos.x * (nearPlaneWidth / *(*graphics).SCR_WIDTH) - (nearPlaneWidth / 2));
		glm::vec3 yOffset = -(*camera).Up * float(mousePos.y * (nearPlaneHeight / *(*graphics).SCR_HEIGHT) - (nearPlaneHeight / 2));

		//these numbers are scaling values to compensate for the oblong window and the clipping pane distance
		glm::vec3 scaler = glm::vec3((1 / 7.7) / 100, (1 / 2.425) / 100, 1);
		mouseRay = glm::normalize(dist*scaler.z + xOffset * scaler.x + yOffset * scaler.y);

		//std::cout << float(x / *(*graphics).SCR_WIDTH) << std::endl;
		//printVector((*camera).pos + mouseRay * 20.0f);
		//printVector(mouseRay);
		//printVector((*graphics).getModel(0).position - (*camera).pos);
	}

	void bindPreviewPiece() {
		//update position
		glm::vec3 displacement = glm::vec3(9.0f, 4.5f, 15.0f);

		previewPiece.asset->setPosition((*camera).pos + displacement.z * (*camera).Front + displacement.x * (*camera).Right + displacement.y * (*camera).Up);
		//printVector((*camera).pos);

		//update rotation
		glm::vec3 rotationDisplacement = glm::vec3(1.0f, 2.0f, 3.0f);
		previewPiece.asset->setRotation(previewPiece.asset->rotation + rotationDisplacement);
	}

	//set callbacks
	void setWinCallback(void f (Piece::Color)) {
		winCallback = f;
	}

	void setPiecePlaceCallback(void f (Piece::Color, glm::vec3)) {
		placePieceCallback = f;
	}

	//utility
	void printVector(glm::vec3 vec) {
		std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
	}
};

#endif