#include "pch.h"
#include "WireframeShader.h"
#include "Sail/Application.h"

WireframeShader::WireframeShader()
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

	setWireframe(true);

	// Finish the shader creation
	finish();

	// Done with the blobs, release them
	/*Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);*/
}
WireframeShader::~WireframeShader() {
}

void WireframeShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void WireframeShader::bind() {

	// Call parent to bind shaders
	Shader::bind();

}
