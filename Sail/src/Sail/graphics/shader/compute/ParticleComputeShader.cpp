#include "pch.h"
#include "ParticleComputeShader.h"

const Shader::ComputeSettings* ParticleComputeShader::getComputeSettings() const {
	return &m_settings;
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
