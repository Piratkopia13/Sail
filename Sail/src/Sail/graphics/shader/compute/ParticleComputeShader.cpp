#include "ParticleComputeShader.h"


const Shader::ComputeSettings* ParticleComputeShader::getComputeSettings() const {
	return &m_settings;
}

std::pair<std::string, void*> ParticleComputeShader::getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) {
	auto& thisInput = static_cast<ParticleComputeShader::Input&>(input);
	switch (index) {
	case 0:
		return { "input", thisInput.inputCB };
	}
	
	Logger::Error("Tried to get compute input from unknown index - " + std::to_string(index));
	return { "", nullptr };
}

Shader::ComputeShaderOutput* ParticleComputeShader::getComputeOutput() {
	return &m_output;
}

ParticleComputeShader::ParticleComputeShader() 
	: Shader("compute/ParticleComputeShader.hlsl") {

	// Finish the shader creation
	finish();

	// Specify dispatch requirements
	m_settings.usesCBV_SRV_UAV = true;
	m_settings.numInputTextures = 0;
	m_settings.numOutputTextures = 0;
	m_settings.threadGroupXScale = 1.f;
}

ParticleComputeShader::~ParticleComputeShader() {

}
