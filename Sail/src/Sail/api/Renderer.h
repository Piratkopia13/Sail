#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

class Mesh;
class Material;
class Camera;
class LightSetup;
class Environment;
class Shader;
class ResourceManager;
class PipelineStateObject;

class Renderer {
public:
	enum Type {
		FORWARD,
		DEFERRED,
		RAYTRACED,
		//TILED
	};
	// TODO: some of these flags are explicitly used for only one renderer, consider changing how flags work to make it more general
	enum PresentFlag {
		Default = 1 << 0,
		SkipPreparation = 1 << 1,
		SkipRendering = 1 << 2,
		SkipExecution = 1 << 3,
		SkipDeferredShading = 1 << 4
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
		glm::mat4 transform;
		Material* material;
		unsigned int materialIndex = 0;
	};

	typedef std::vector<Renderer::RenderCommand>& RenderCommandList;

public:
	static Renderer* Create(Renderer::Type type);
	Renderer();
	virtual ~Renderer() {}

	virtual void begin(Camera* camera, Environment* environment);
	virtual void submit(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& modelMatrix);
	virtual void setLightSetup(LightSetup* lightSetup);
	virtual void useDepthBuffer(void* buffer, void* cmdList);
	virtual void* getDepthBuffer();
	virtual void end();
	virtual void* present(PresentFlag flags, void* skippedPrepCmdList = nullptr) = 0;

protected:
	std::unordered_map<PipelineStateObject*, std::vector<RenderCommand>> commandQueue; // Sorted by PSOs
	std::vector<RenderCommand> commandQueueCustom; // Unsorted, without PSOs
	Camera* camera = nullptr;
	Environment* environment = nullptr;
	LightSetup* lightSetup = nullptr;

private:
	ResourceManager* m_resman;

};

// Operators to use enums as bit flags
inline Renderer::PresentFlag operator|(Renderer::PresentFlag a, Renderer::PresentFlag b) {
	return static_cast<Renderer::PresentFlag>(static_cast<int>(a) | static_cast<int>(b));
}
inline Renderer::DXRRenderFlag operator|(Renderer::DXRRenderFlag a, Renderer::DXRRenderFlag b) {
	return static_cast<Renderer::DXRRenderFlag>(static_cast<int>(a) | static_cast<int>(b));
}