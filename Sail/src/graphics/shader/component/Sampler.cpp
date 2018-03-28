#include "Sampler.h"
#include "../../../api/Application.h"

namespace ShaderComponent {

	Sampler::Sampler(D3D11_TEXTURE_ADDRESS_MODE addressMode, D3D11_FILTER filter, BIND_SHADER bindShader, UINT slot)
		: m_bindShader(bindShader)
		, m_slot(slot)
	{

		// Set up sampler
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		desc.Filter = filter;
		desc.MipLODBias = 0.f;
		desc.MaxAnisotropy = 0;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		Application::getInstance()->getAPI()->getDevice()->CreateSamplerState(&desc, &m_samplerState);

	}

	Sampler::~Sampler() {
		Memory::safeRelease(m_samplerState);
	}

	void Sampler::bind() {
		if (m_bindShader & ShaderComponent::VS)
			Application::getInstance()->getAPI()->getDeviceContext()->VSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::HS)
			Application::getInstance()->getAPI()->getDeviceContext()->HSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::DS)
			Application::getInstance()->getAPI()->getDeviceContext()->DSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::PS)
			Application::getInstance()->getAPI()->getDeviceContext()->PSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::GS)
			Application::getInstance()->getAPI()->getDeviceContext()->GSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::CS)
			Application::getInstance()->getAPI()->getDeviceContext()->CSSetSamplers(m_slot, 1, &m_samplerState);
	}

}