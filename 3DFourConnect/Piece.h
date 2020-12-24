#ifndef PIECE_H
#define PIECE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.h>

#include <iostream>

//graphics tools
#include "GraphicsEngine.h"
#include "Camera.h"
#include "Model.h"

const char* RedModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\redball\\redball.obj";
const char* BlueModelPath = "C:\\Users\\Erik\\source\\repos\\3DFourConnect\\3DFourConnect\\resources\\objects\\blueball\\blueball.obj";

//if the Piece has type NONE then it is an empty space.
class Piece {
public:
	GraphicsEngine *graphics;
	Model *model;

	float colliderRadius = 0.3f;

	//whether the piece is blue or red
	enum Color { NONE = 0,  RED = 1, BLUE = 2 };
	Color type;

	Piece() {
		type = Color::NONE;
	}

	Piece(GraphicsEngine *graphics, Color type, glm::vec3 pos) {
		this->graphics = graphics;
		this->type = type;

		//make models based on color, set the position, and set the pointer to the model.
		if (this->type == Color::RED) {
			(*(this->graphics)).addModel(model, RedModelPath, pos, glm::vec3(0), glm::vec3(1));
		}
		else if (this->type == Color::BLUE) {
			(*(this->graphics)).addModel(model, BlueModelPath, pos, glm::vec3(0), glm::vec3(1));
		}
	}
};

#endif