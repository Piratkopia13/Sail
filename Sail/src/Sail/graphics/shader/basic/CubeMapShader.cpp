#include "pch.h"
#include "CubemapShader.h"
#include "Sail/Application.h"

CubemapShader::CubemapShader()
	: Shader("CubemapShader.hlsl")
	, m_clippingPlaneHasChanged(false) {
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	setMaterialType(Material::TEXTURES);

	auto* pipeline = getPipeline();
	pipeline->setCullMode(GraphicsAPI::FRONTFACE);
	pipeline->setDepthMask(GraphicsAPI::BUFFER_DISABLED);

	// Finish the shader creation
	finish();
}

CubemapShader::~CubemapShader() { }

void CubemapShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void CubemapShader::bind() {
	// Call parent to bind shaders
	Shader::bind();
}
