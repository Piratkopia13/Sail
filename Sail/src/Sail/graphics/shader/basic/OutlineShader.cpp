#include "pch.h"
#include "OutlineShader.h"
#include "Sail/Application.h"

OutlineShader::OutlineShader()
	: Shader("OutlineShader.hlsl")
	, m_clippingPlaneHasChanged(false) {
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	setMaterialType(Material::OUTLINE);

	auto* pipeline = getPipeline();
	pipeline->setCullMode(GraphicsAPI::FRONTFACE);
	//pipeline->setDepthMask(GraphicsAPI::BUFFER_DISABLED);

	// Finish the shader creation
	finish();
}

OutlineShader::~OutlineShader() { }

void OutlineShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void OutlineShader::bind() {
	// Call parent to bind shaders
	Shader::bind();
}
