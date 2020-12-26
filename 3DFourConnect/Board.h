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

const char* BoardPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\3dfourconnect\\3dfourconnectFIXED.obj";

class Board {
public:
	GraphicsEngine *graphics;
	Asset *asset;

	Piece data[4][4][4];

	glm::vec3 piecePosScalar = glm::vec3(5, 7, 5);
	glm::vec3 bottomLeft = glm::vec3(-1.5, 0.10, -1.5);

	Board() {

	}

	Board(GraphicsEngine &graphics, glm::vec3 pos) {
		this->graphics = &graphics;

		asset = new Asset((*(this->graphics)).getModel("3dfourconnectFIXED.obj"), pos, glm::vec3(0), glm::vec3(1));
		(*(this->graphics)).addAsset(asset);
		//std::cout << model << std::endl;
	}

	//range (0-3) inclusive (integers)
	bool addPiece(Piece::Color color, int x, int y, int z) {
		//check if another piece is already there
		if (data[x][y][z].type != Piece::Color::NONE) {
			return false;
		}

		//since the origin of the board is at the bottom center, we just find the bottom left corner and work relatively.
		glm::vec3 pos = asset->position + (bottomLeft + glm::vec3(x,y,z)) * piecePosScalar;
		data[x][y][z] = Piece(graphics, color, pos);

		return true;
	}

	void clearBoard() {
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					(*graphics).removeAsset(data[x][y][z].asset);
					data[x][y][z] = Piece();
				}
			}
		}
	}

	//since the model's origin is at the bottom center, we just get the conversion rate of the y axis and multiply it by 1.5 since there are 4 plates
	glm::vec3 getCenter() {
		//return (*model).position + glm::vec3(0, piecePosScalar.y * 1.5, 0);
		return glm::vec3(0, piecePosScalar.y * 1.5, 0);
	}
};

#endif