#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <vector>
#include <iostream>

class Model {
public:
	std::vector<std::vector<unsigned int>> VBO;
	std::vector<std::vector<unsigned int>> VAO;

	Model() {
		
	}
};

#endif