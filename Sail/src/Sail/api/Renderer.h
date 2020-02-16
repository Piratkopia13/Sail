#pragma once

#include <glm/glm.hpp>
#include <vector>

class Mesh;
class Material;
class Camera;
class Model;
class LightSetup;
class Environment;
class Shader;

class Renderer {
public:
	enum Type {
		FORWARD,
		DEFERRED,
		//RAYTRACED
		//TILED
	};
	enum RenderFlag {
		Default = 1 << 0,
		SkipPreparation = 1 << 1,
		SkipRendering = 1 << 2,
		SkipExecution = 1 << 3
	};

public:
	static Renderer* Create(Renderer::Type type);
	virtual ~Renderer() {}

	virtual void begin(Camera* camera, Environment* environment);
	void submit(Model* model, Shader* shader, Material* material, const glm::mat4& modelMatrix);
	virtual void submit(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& modelMatrix);
	virtual void setLightSetup(LightSetup* lightSetup);
	virtual void useDepthBuffer(void* buffer, void* cmdList);
	virtual void* getDepthBuffer();
	virtual void end();
	virtual void* present(RenderFlag flags, void* skippedPrepCmdList = nullptr) = 0;

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

inline Renderer::RenderFlag operator|(Renderer::RenderFlag a, Renderer::RenderFlag b) {
	return static_cast<Renderer::RenderFlag>(static_cast<int>(a) | static_cast<int>(b));
}