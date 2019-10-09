#include "pch.h"
#include "AnimationUpdateComputeShader.h"

const Shader::ComputeSettings* AnimationUpdateComputeShader::getComputeSettings() const {
	return &m_settings;
}

std::pair<std::string, void*> AnimationUpdateComputeShader::getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) {
	auto& thisInput = static_cast<AnimationUpdateComputeShader::Input&>(input);
	switch (index) {
	//case 0:
		//return { "input", thisInput.inputTexture };
	}
	Logger::Error("Tried to get compute input from unknown index - " + std::to_string(index));
	return { "", nullptr };
}

RenderableTexture* AnimationUpdateComputeShader::getComputeOutputForIndex(Shader::ComputeShaderOutput& output, unsigned int index) {
	auto& thisOutput = static_cast<AnimationUpdateComputeShader::Output&>(output);
	switch (index) {
	//case 0:
		//return thisOutput.outputTexture;
	}
	Logger::Error("Tried to get compute output from unknown index - " + std::to_string(index));
	return nullptr;
}

Shader::ComputeShaderOutput* AnimationUpdateComputeShader::getComputeOutput() {
	return &m_output;
}

AnimationUpdateComputeShader::AnimationUpdateComputeShader()
	: Shader("compute/AnimationUpdateComputeShader.hlsl") {
	// Finish the shader creation
	finish();

	// Specify dispatch requirements
	m_settings.usesCBV_SRV_UAV = true;
	m_settings.numInputTextures = 0;
	m_settings.numOutputTextures = 0;

}
AnimationUpdateComputeShader::~AnimationUpdateComputeShader() {}