#include "pch.h"
#include "GenerateMipsComputeShader.h"

const Shader::ComputeSettings* GenerateMipsComputeShader::getComputeSettings() const {
	return &m_settings;
}

std::pair<std::string, void*> GenerateMipsComputeShader::getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) {
	auto& thisInput = static_cast<GenerateMipsComputeShader::Input&>(input);
	switch (index) {
	case 0:
		return { "input", thisInput.inputTexture };
	}
	Logger::Error("Tried to get compute input from unknown index - " + std::to_string(index));
	return { "", nullptr };
}

RenderableTexture* GenerateMipsComputeShader::getComputeOutputForIndex(Shader::ComputeShaderOutput& output, unsigned int index) {
	auto& thisOutput = static_cast<GenerateMipsComputeShader::Output&>(output);
	switch (index) {
	case 0:
		return thisOutput.outputTexture;
	}
	Logger::Error("Tried to get compute output from unknown index - " + std::to_string(index));
	return nullptr;
}

Shader::ComputeShaderOutput* GenerateMipsComputeShader::getComputeOutput() {
	return &m_output;
}

GenerateMipsComputeShader::GenerateMipsComputeShader()
	: Shader("compute/GenerateMipsCS.hlsl")
	, m_clippingPlaneHasChanged(false) {
	// Finish the shader creation
	finish();

	// Specify dispatch requirements
	m_settings.usesCBV_SRV_UAV = false;
	m_settings.numInputTextures = 0;
	m_settings.numOutputTextures = 0;
	m_settings.threadGroupXScale = 1.0f / 8.0f;
	m_settings.threadGroupYScale = 1.0f / 8.0f;

}
GenerateMipsComputeShader::~GenerateMipsComputeShader() { }