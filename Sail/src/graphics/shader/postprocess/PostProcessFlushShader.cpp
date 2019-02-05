#include "pch.h"
#include "PostProcessFlushShader.h"

using namespace DirectX;
using namespace SimpleMath;

PostProcessFlushShader::PostProcessFlushShader() 
	: ShaderSet("postprocess/PostProcessFlushShader.hlsl")
{

	// Create the input layout
	inputLayout.push<Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);

}
PostProcessFlushShader::~PostProcessFlushShader() {
}

void PostProcessFlushShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

}
