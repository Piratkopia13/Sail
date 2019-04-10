#pragma once

#include <glm/glm.hpp>

class Mesh;

class Renderer {
public:
	Renderer() {}
	virtual ~Renderer() {}

	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix) = 0;
	virtual void draw() const = 0;

protected:

private:

};