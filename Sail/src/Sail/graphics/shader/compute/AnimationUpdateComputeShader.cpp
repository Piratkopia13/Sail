#include "pch.h"
#include "AnimationUpdateComputeShader.h"

const Shader::ComputeSettings* AnimationUpdateComputeShader::getComputeSettings() const {
	return &m_settings;
}

std::pair<std::string, void*> AnimationUpdateComputeShader::getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) {
	SAIL_LOG_ERROR("This shader has no compute inputs");
	return { "", nullptr };
}

RenderableTexture* AnimationUpdateComputeShader::getComputeOutputForIndex(Shader::ComputeShaderOutput& output, unsigned int index) {
	SAIL_LOG_ERROR("This shader has no compute outputs");
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
	m_settings.threadGroupXScale = 1.f;

}
AnimationUpdateComputeShader::~AnimationUpdateComputeShader() {}