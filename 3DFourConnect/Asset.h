#ifndef ASSET_H
#define ASSET_H

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
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"

//the physical instance of the model with transforms
class Asset {
public:
	Model* model;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Asset() {

	}

	Asset(Model *model) {
		this->model = model;

		position = glm::vec3(0.0f);
		rotation = glm::vec3(0.0f);
		scale = glm::vec3(1.0f);
	}

	Asset(Model *model, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		this->model = model;

		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}

	void setPosition(glm::vec3 position) {
		this->position = position;
	}

	void setRotation(glm::vec3 rotation) {
		this->rotation = rotation;

		if (this->rotation.x > 360) {
			this->rotation.x = this->rotation.x - 360;
		}
		if (this->rotation.x < 0) {
			this->rotation.x = 360 + this->rotation.x;
		}

		if (this->rotation.y > 360) {
			this->rotation.y = this->rotation.y - 360;
		}
		if (this->rotation.y < 0) {
			this->rotation.y = 360 + this->rotation.y;
		}

		if (this->rotation.z > 360) {
			this->rotation.z = this->rotation.z - 360;
		}
		if (this->rotation.z < 0) {
			this->rotation.z = 360 + this->rotation.z;
		}
	}

	void setScale(glm::vec3 scale) {
		this->scale = scale;
	}
};

#endif