#pragma once

#include <glm/glm.hpp>

class Mesh;
class Camera;
class Model;
class LightSetup;
class RenderableTexture;

class Renderer {
public:
	enum Type {
		FORWARD
		//DEFERRED
		//RAYTRACED
		//TILED
	};
public:
	static Renderer* Create(Renderer::Type type);
	virtual ~Renderer() {}

	virtual void begin(Camera* camera);
	void submit(Model* model, const glm::mat4& modelMatrix);
	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix);
	virtual void setLightSetup(LightSetup* lightSetup);
	virtual void end();
	virtual void present(RenderableTexture* output = nullptr) = 0;

protected:
	struct RenderCommand {
		RenderCommand(Mesh* mesh, const glm::mat4& transform)
			: mesh(mesh)
			, transform(transform)
		{}

		Mesh* mesh;
		glm::mat4 transform;
	};

	std::vector<RenderCommand> commandQueue;
	Camera* camera;
	LightSetup* lightSetup;

};