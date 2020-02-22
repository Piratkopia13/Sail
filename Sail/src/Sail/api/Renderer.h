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
	enum PresentFlag {
		Default = 1 << 0,
		SkipPreparation = 1 << 1,
		SkipRendering = 1 << 2,
		SkipExecution = 1 << 3
	};
	enum DXRRenderFlag {
		MESH_DYNAMIC			= 1 << 0,	// Vertices may change
		MESH_STATIC				= 1 << 1,	// Vertices will never change
		MESH_TRANSPARENT		= 1 << 2,	// Should be rendered see-through
		MESH_HERO				= 1 << 3	// Mesh takes up a relatively large area of the screen 
	};

	struct RenderCommand {
		DXRRenderFlag dxrFlags;
		Mesh* mesh;
		Shader* shader;
		glm::mat4 transform;
		Material* material;
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
	virtual void* present(PresentFlag flags, void* skippedPrepCmdList = nullptr) = 0;

protected:
	std::vector<RenderCommand> commandQueue;
	Camera* camera;
	Environment* environment;
	LightSetup* lightSetup;

};

// Operators to use enums as bit flags
inline Renderer::PresentFlag operator|(Renderer::PresentFlag a, Renderer::PresentFlag b) {
	return static_cast<Renderer::PresentFlag>(static_cast<int>(a) | static_cast<int>(b));
}
inline Renderer::DXRRenderFlag operator|(Renderer::DXRRenderFlag a, Renderer::DXRRenderFlag b) {
	return static_cast<Renderer::DXRRenderFlag>(static_cast<int>(a) | static_cast<int>(b));
}