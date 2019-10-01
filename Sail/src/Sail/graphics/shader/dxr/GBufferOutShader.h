#pragma once

#include "Sail/graphics/shader/Shader.h"

class GBufferOutShader : public Shader {
public:
	GBufferOutShader();
	~GBufferOutShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;

private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;

};
