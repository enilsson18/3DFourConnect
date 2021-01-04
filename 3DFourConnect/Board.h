#ifndef BOARD_H
#define BOARD_H

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
#include <vector>

//graphics tools
#include "GraphicsEngine.h"
#include "Camera.h"
#include "Model.h"

//other pieces
#include "Piece.h"

//Networking
#include "Tools.h"

class Board {
public:
	GraphicsEngine *graphics = nullptr;
	Asset *asset = nullptr;

	Piece data[4][4][4];

	glm::vec3 piecePosScalar = glm::vec3(5, 7, 5);
	glm::vec3 bottomLeft = glm::vec3(-1.5, 0.10, -1.5);

	Board() {
		graphics = nullptr;

		asset = new Asset(nullptr, glm::vec3(0), glm::vec3(0), glm::vec3(1));

		//do initial setup for board
		clearBoard();
	}

	Board(GraphicsEngine &graphics, glm::vec3 pos) {
		this->graphics = &graphics;

		asset = new Asset((*(this->graphics)).getModel("3dfourconnectFIXED.obj"), pos, glm::vec3(0), glm::vec3(1));
		(*(this->graphics)).addAsset(asset);
		
		//do initial setup for board
		clearBoard();
	}

	//range (0-3) inclusive (integers)
	bool addPiece(Piece::Color color, int x, int y, int z) {
		//check if another piece is already there
		if (data[x][y][z].type != Piece::Color::NONE) {
			return false;
		}

		//remove old asset
		graphics->removeAsset(data[x][y][z].asset);
		//since the origin of the board is at the bottom center, we just find the bottom left corner and work relatively.
		glm::vec3 pos = getPiecePosFromCoord(x,y,z);
		data[x][y][z] = Piece(graphics, color, pos);

		return true;
	}

	glm::vec3 getPiecePosFromCoord(int x, int y, int z) {
		return asset->position + (bottomLeft + glm::vec3(x, y, z)) * piecePosScalar;
	}

	void clearBoard() {
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					if (graphics != nullptr) {
						graphics->removeAsset(data[x][y][z].asset);
					}
					//since the origin of the board is at the bottom center, we just find the bottom left corner and work relatively.
					glm::vec3 pos = getPiecePosFromCoord(x, y, z);
					data[x][y][z] = Piece(graphics, Piece::Color::NONE, pos);
				}
			}
		}
	}

	bool fullBoard() {
		int pieces = 0;
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					if (data[x][y][z].type != Piece::Color::NONE) {
						pieces += 1;
					}
				}
			}
		}

		if (pieces == 4 * 4 * 4) {
			return true;
		}
		return false;
	}

	//utility
	//since the model's origin is at the bottom center, we just get the conversion rate of the y axis and multiply it by 1.5 since there are 4 plates
	glm::vec3 getCenter() {
		//return (*model).position + glm::vec3(0, piecePosScalar.y * 1.5, 0);
		return glm::vec3(0, piecePosScalar.y * 1.5, 0);
	}

	//sets all the board positons and current to whatever the datapacked says
	void setBoardToData(DataPacket *datapacket) {
		//std::cout << "set board to data" << std::endl;
		clearBoard();
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					if (datapacket->board[x][y][z] == 0) {
						addPiece(Piece::Color::NONE, x, y, z);
					}
					if (datapacket->board[x][y][z] == 1) {
						addPiece(Piece::Color::RED, x, y, z);
					}
					if (datapacket->board[x][y][z] == 2) {
						addPiece(Piece::Color::BLUE, x, y, z);
					}
				}
			}
		}
	}
};

#endif