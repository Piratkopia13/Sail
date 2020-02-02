#pragma once

#include "../utils/Utils.h"
#include "Sail/events/Events.h"

class Window;

class GraphicsAPI : public IEventListener {
public:
	enum DepthMask {
		NO_MASK,
		WRITE_MASK,
		BUFFER_DISABLED
	};
	enum Culling {
		NO_CULLING,
		FRONTFACE,
		BACKFACE
	};
	enum Blending {
		NO_BLENDING,
		ALPHA,
		ADDITIVE
	};

public:
	static GraphicsAPI* Create();
	GraphicsAPI();;
	virtual ~GraphicsAPI();;

	virtual bool init(Window* window) = 0;
	virtual void clear(const glm::vec4& color) = 0;
	virtual void setDepthMask(DepthMask setting) = 0;
	virtual void setFaceCulling(Culling setting) = 0;
	virtual void setBlending(Blending setting) = 0;
	virtual void present(bool vsync = false) = 0;
	virtual unsigned int getMemoryUsage() const = 0;
	virtual unsigned int getMemoryBudget() const = 0;
	virtual void toggleFullscreen() { /* All APIs might not need to implement this */ };

	virtual bool onResize(WindowResizeEvent& event) = 0;
	virtual bool onEvent(Event& event) override;
};