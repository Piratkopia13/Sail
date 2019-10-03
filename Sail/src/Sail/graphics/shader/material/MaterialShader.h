#pragma once

#include <glm/glm.hpp>
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"

class MaterialShader : public Shader {
public:
	MaterialShader();
	~MaterialShader();

	virtual void bind() override;
	virtual void setClippingPlane(const glm::vec4& clippingPlane) override;

private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;

};
