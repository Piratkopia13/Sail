#include "pch.h"
#include "TestComputeShader.h"
#include "Sail/Application.h"

TestComputeShader::TestComputeShader()
	: Shader("compute/ComputeTest.hlsl")
	, m_clippingPlaneHasChanged(false)
{
	// Finish the shader creation
	finish();

}
TestComputeShader::~TestComputeShader() {
}

void TestComputeShader::bind() {

	// Call parent to bind shaders
	Shader::bind();
	// TODO: create the structuredBuffers and whatever in shaderPipeline and bind in bind()
	

}