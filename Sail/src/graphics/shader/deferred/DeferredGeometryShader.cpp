#include "pch.h"
#include "DeferredGeometryShader.h"

using namespace DirectX;
using namespace SimpleMath;

DeferredGeometryShader::DeferredGeometryShader() 
	: ShaderSet("deferred/GeometryShader.hlsl")
{

	// Create the input layout
	inputLayout.push<Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.push<Vector2>(InputLayout::TEXCOORD, "TEXCOORD", 0);
	inputLayout.push<Vector3>(InputLayout::NORMAL, "NORMAL", 0);
	inputLayout.push<Vector3>(InputLayout::TANGENT, "TANGENT", 0);
	inputLayout.push<Vector3>(InputLayout::BITANGENT, "BINORMAL", 0);
	inputLayout.create(VSBlob);

}
DeferredGeometryShader::~DeferredGeometryShader() {
}

void DeferredGeometryShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();
}
