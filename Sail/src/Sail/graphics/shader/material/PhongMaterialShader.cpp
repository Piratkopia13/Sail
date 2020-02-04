#include "pch.h"
#include "PhongMaterialShader.h"
#include "Sail/Application.h"

PhongMaterialShader::PhongMaterialShader()
	: Shader("PhongMaterialShader.hlsl")
	, m_clippingPlaneHasChanged(false)
{
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	shaderPipeline->getInputLayout().pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	setMaterialType(Material::PHONG);

	// Finish the shader creation
	finish();
}

PhongMaterialShader::~PhongMaterialShader() { }

void PhongMaterialShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void PhongMaterialShader::bind() {
	// Call parent to bind shaders
	Shader::bind();
}
