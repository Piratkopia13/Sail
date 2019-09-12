#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"
#include "../../geometry/Material.h"

class TestComputeShader : public Shader {
public:
	TestComputeShader();
	~TestComputeShader();

	virtual void bind() override;

private:
	glm::vec4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;

};
