#ifndef ASSET_H
#define ASSET_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include "Camera.h"
#include "Model.h"

#include <vector>
#include <iostream>

//assets are collections of or single models.
//all drawing is done within the asset.
class Asset {
public:
	std::vector<Model> models;

	Asset() {
		models = std::vector<Model>();
	}

	void draw(glm::mat4 projection, Camera &camera) {

	}
};

#endif