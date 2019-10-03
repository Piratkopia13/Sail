#pragma once

#include <glm/glm.hpp>
#include "Sail/events/Events.h"

class Mesh;
class Camera;
class Model;
class LightSetup;
class RenderableTexture;
class PostProcessPipeline;

class Renderer : public IEventListener {
public:
	enum Type {
		FORWARD,
		DEFERRED,
		RAYTRACED,
		GBUFFER,
		HYBRID
	};

	enum RenderFlag {
		MESH_DYNAMIC = 1 << 0,		// Vertices may change
		MESH_STATIC = 1 << 1,		// Vertices will never change
		MESH_TRANSPARENT = 1 << 2,	// Should be rendered see-through
		MESH_HERO = 1 << 3			// Mesh takes up a relatively large area of the screen 
	};	
	struct RenderCommand {
		Mesh* mesh;
		glm::mat4 transform; // TODO: find out why having a const ptr here doesnt work
		RenderFlag flags = MESH_STATIC;
		std::vector<bool> hasUpdatedSinceLastRender;
	};
public:
	static Renderer* Create(Renderer::Type type);
	virtual ~Renderer() {}

	virtual void begin(Camera* camera);
	void submit(Model* model, const glm::mat4& modelMatrix, RenderFlag flags);
	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags);
	virtual void setLightSetup(LightSetup* lightSetup);
	virtual void end();
	virtual void present(PostProcessPipeline* postProcessPipeline = nullptr, RenderableTexture* output = nullptr) = 0;
	virtual bool onEvent(Event& event) override { return true; };

protected:
	std::vector<RenderCommand> commandQueue;
	Camera* camera;
	LightSetup* lightSetup;

};

// Operators to use RenderFlags enum as bit flags
inline enum Renderer::RenderFlag operator|(const enum Renderer::RenderFlag a, const enum Renderer::RenderFlag b) {
	return static_cast<enum Renderer::RenderFlag>(static_cast<int>(a) | static_cast<int>(b));
}
inline enum Renderer::RenderFlag operator|=(const enum Renderer::RenderFlag a, const enum Renderer::RenderFlag b) {
	return static_cast<enum Renderer::RenderFlag>(static_cast<int>(a) | static_cast<int>(b));
}