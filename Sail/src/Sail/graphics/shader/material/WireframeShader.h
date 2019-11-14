#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"

class WireframeShader : public Shader {
public:
	WireframeShader();
	~WireframeShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;

private:


private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;
};
