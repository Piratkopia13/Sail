#include "pch.h"
#include "MaterialShader.h"
#include "Sail/Application.h"

MaterialShader::MaterialShader()
	: Shader("MaterialShader.hlsl")
	, m_clippingPlaneHasChanged(false)
{
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	// Finish the shader creation
	finish();

	// Done with the blobs, release them
	/*Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);*/
}
MaterialShader::~MaterialShader() {
}

void MaterialShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void MaterialShader::bind() {

	// Call parent to bind shaders
	Shader::bind();

}
