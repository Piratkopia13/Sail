#include "pch.h"
#include "GenerateMipsComputeShader.h"

const Shader::ComputeSettings* GenerateMipsComputeShader::getComputeSettings() const {
	return &m_settings;
}

GenerateMipsComputeShader::GenerateMipsComputeShader()
	: Shader("compute/GenerateMipsCS.hlsl")
	, m_clippingPlaneHasChanged(false) {
	// Finish the shader creation
	finish();

	// Specify dispatch requirements
	m_settings.threadGroupXScale = 1.0f / 8.0f;
	m_settings.threadGroupYScale = 1.0f / 8.0f;

}
GenerateMipsComputeShader::~GenerateMipsComputeShader() { }