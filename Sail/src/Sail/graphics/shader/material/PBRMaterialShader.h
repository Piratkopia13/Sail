#pragma once

#include "Sail/graphics/shader/Shader.h"

class PBRMaterialShader : public Shader {
public:
	PBRMaterialShader();
	~PBRMaterialShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;
private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;
};
