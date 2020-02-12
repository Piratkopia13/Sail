#pragma once

#include "Sail/api/shader/Shader.h"

class PhongMaterialShader : public Shader {
public:
	PhongMaterialShader();
	~PhongMaterialShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;
private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;
};
