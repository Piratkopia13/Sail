#include "pch.h"
#include "PBRMaterialShader.h"
#include "Sail/Application.h"

PBRMaterialShader::PBRMaterialShader()
	: Shader("PBRMaterialShader.hlsl")
	, m_clippingPlaneHasChanged(false)
{
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	setMaterialType(Material::PBR);

	// Finish the shader creation
	finish();
}

PBRMaterialShader::~PBRMaterialShader() { }

void PBRMaterialShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void PBRMaterialShader::bind() {
	// Call parent to bind shaders
	Shader::bind();
}
