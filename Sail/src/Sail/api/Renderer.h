#pragma once

#include <glm/glm.hpp>
#include <vector>

class Mesh;
class Material;
class Camera;
class Model;
class LightSetup;
class RenderableTexture;
class Environment;
class Shader;

class Renderer {
public:
	enum Type {
		FORWARD,
		DEFERRED
		//RAYTRACED
		//TILED
	};
public:
	static Renderer* Create(Renderer::Type type);
	virtual ~Renderer() {}

	virtual void begin(Camera* camera, Environment* environment);
	void submit(Model* model, Shader* shader, Material* material, const glm::mat4& modelMatrix);
	virtual void submit(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& modelMatrix);
	virtual void setLightSetup(LightSetup* lightSetup);
	virtual void end();
	virtual void present(RenderableTexture* output = nullptr) = 0;

protected:
	struct RenderCommand {
		RenderCommand(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& transform)
			: mesh(mesh)
			, shader(shader)
			, material(material)
			, transform(transform)
		{}

		Mesh* mesh;
		Shader* shader;
		glm::mat4 transform;
		Material* material;
	};

	std::vector<RenderCommand> commandQueue;
	Camera* camera;
	Environment* environment;
	LightSetup* lightSetup;

};