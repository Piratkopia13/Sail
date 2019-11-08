#include "pch.h"
#include "ShadePassShader.h"

ShadePassShader::ShadePassShader()
	: Shader("dxr/shading/ShadePass.hlsl")
{
	// Create the input layout
	shaderPipeline->getInputLayout().pushVec3(InputLayout::POSITION, "POSITION", 0);
	shaderPipeline->getInputLayout().create(shaderPipeline->getVsBlob());

	shaderPipeline->useDepthStencil(false);

	// Finish the shader creation
	finish();
}
ShadePassShader::~ShadePassShader() {

}

void ShadePassShader::bind() {
	// Call parent to bind shaders
	Shader::bind();
}
