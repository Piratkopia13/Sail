#include "pch.h"
#include "DX11ShaderPipeline.h"
#include "Sail/Application.h"
#include "../DX11API.h"

void ShaderPipeline::Bind(ShaderPipeline* instance) {
	
	// Don't bind if already bound
	// This is to cut down on shader state changes
	if (CurrentlyBoundShader == instance)
		return;

	//auto* devCon = Application::getInstance()->getAPI()->getDeviceContext();

	// TODO: emulate pipeline with individual shader objects
	//if (instance.vsBlob)	m_vs->bind();
	////else		devCon->VSSetShader(nullptr, 0, 0);
	//if (instance.gsBlob)	m_gs->bind();
	////else		devCon->GSSetShader(nullptr, 0, 0);
	//if (instance.psBlob)	m_ps->bind();
	////else		devCon->PSSetShader(nullptr, 0, 0);
	//if (instance.dsBlob)	m_ds->bind();
	////else		devCon->DSSetShader(nullptr, 0, 0);
	//if (instance.hsBlob)	m_hs->bind();
	////else		devCon->HSSetShader(nullptr, 0, 0);

	for (auto& it : instance->m_parsedData.cBuffers) {
		it.cBuffer->bind();
	}
	for (auto& it : instance->m_parsedData.samplers) {
		it.sampler->bind();
	}

	// Set input layout as active
	instance->inputLayout->bind();

	// Set this shader as bound
	CurrentlyBoundShader = instance;

}

void* ShaderPipeline::CompileShader(const std::string& source, ShaderComponent::BIND_SHADER shaderType) {

	std::string entryPoint = "VSMain";
	std::string shaderVersion = "vs_5_0";
	switch (shaderType) {
	case ShaderComponent::VS:
		entryPoint = "VSMain";
		shaderVersion = "vs_5_0";
		break;
	case ShaderComponent::PS:
		entryPoint = "PSMain";
		shaderVersion = "ps_5_0";
		break;
	case ShaderComponent::GS:
		entryPoint = "GSMain";
		shaderVersion = "gs_5_0";
		break;
	case ShaderComponent::CS:
		entryPoint = "CSMain";
		shaderVersion = "cs_5_0";
		break;
	case ShaderComponent::DS:
		entryPoint = "DSMain";
		shaderVersion = "ds_5_0";
		break;
	case ShaderComponent::HS:
		entryPoint = "HSMain";
		shaderVersion = "hs_5_0";
		break;
	}

	ID3D10Blob* shader = nullptr;
	ID3D10Blob* errorMsg;
	if (FAILED(D3DCompile(source.c_str(), source.size(), DEFAULT_SHADER_LOCATION.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderVersion.c_str(), D3DCOMPILE_DEBUG, 0, &shader, &errorMsg))) {
		OutputDebugString(L"\n Failed to compile shader\n\n");

		char* msg = (char*)(errorMsg->GetBufferPointer());

		std::stringstream ss;
		ss << "Failed to compile shader (" << entryPoint << ", " << shaderVersion << ")\n";

		for (size_t i = 0; i < errorMsg->GetBufferSize(); i++) {
			ss << msg[i];
		}
		Logger::Error(ss.str());
	}

	return shader;
}
void ShaderPipeline::SetTexture2D(ShaderPipeline* instance, const std::string& name, void* handle) {
	UINT slot = instance->findSlotFromName(name, instance->m_parsedData.textures);
	Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&handle);
}