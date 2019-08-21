#include "pch.h"
#include "DeferredGeometryShader.h"

DeferredGeometryShader::DeferredGeometryShader() 
	: ShaderSet("deferred/GeometryShader.hlsl")
{

	// Create the input layout
	inputLayout.push<glm::vec3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.push<glm::vec2>(InputLayout::TEXCOORD, "TEXCOORD", 0);
	inputLayout.push<glm::vec3>(InputLayout::NORMAL, "NORMAL", 0);
	inputLayout.push<glm::vec3>(InputLayout::TANGENT, "TANGENT", 0);
	inputLayout.push<glm::vec3>(InputLayout::BITANGENT, "BINORMAL", 0);
	inputLayout.create(VSBlob);

}
DeferredGeometryShader::~DeferredGeometryShader() {
}

void DeferredGeometryShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();
}
