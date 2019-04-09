#pragma once

#include <SimpleMath.h>

class Mesh;

class Renderer {
public:
	Renderer() {}
	virtual ~Renderer() {}

	virtual void submit(Mesh* mesh, const DirectX::SimpleMath::Matrix& modelMatrix) = 0;
	virtual void draw() const = 0;

protected:

private:

};