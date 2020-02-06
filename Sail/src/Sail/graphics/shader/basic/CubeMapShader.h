#pragma once

#include "Sail/graphics/shader/Shader.h"

class CubemapShader : public Shader {
public:
	CubemapShader();
	~CubemapShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;
private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;
};
