#include "pch.h"
#include "GBufferOutShaderNoDepth.h"
#include "Sail/Application.h"
#include "API/DX12/shader/DX12ShaderPipeline.h"

GBufferOutShaderNoDepth::GBufferOutShaderNoDepth()
	: Shader("dxr/GBufferParticles.hlsl")
	, m_clippingPlaneHasChanged(false) {
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	shaderPipeline->setNumRenderTargets(3);

	// Disable depth writing
	getPipeline()->enableDepthWriting(false);
	getPipeline()->setBlending(GraphicsAPI::ADDITIVE);

	// Finish the shader creation
	finish();
}
GBufferOutShaderNoDepth::~GBufferOutShaderNoDepth() { }

void GBufferOutShaderNoDepth::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void GBufferOutShaderNoDepth::bind() {

	// Call parent to bind shaders
	Shader::bind();

}
