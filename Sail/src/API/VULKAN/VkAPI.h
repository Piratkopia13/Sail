#pragma once
#include "Sail/api/GraphicsAPI.h"

class VkAPI : public GraphicsAPI {
public:
	VkAPI();

	bool init(Window* window) override;
	void clear(const glm::vec4& color) override;
	void setDepthMask(DepthMask setting) override;
	void setFaceCulling(Culling setting) override;
	void setBlending(Blending setting) override;
	void present(bool vsync = false) override;
	unsigned int getMemoryUsage() const override;
	unsigned int getMemoryBudget() const override;
	bool onResize(WindowResizeEvent& event) override;

};