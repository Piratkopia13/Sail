#include "pch.h"
#include "PostProcessFlushShader.h"

PostProcessFlushShader::PostProcessFlushShader() 
	: ShaderSet("postprocess/PostProcessFlushShader.hlsl")
{

	// Create the input layout
	inputLayout.push<glm::vec3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);

}
PostProcessFlushShader::~PostProcessFlushShader() {
}

void PostProcessFlushShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

}
