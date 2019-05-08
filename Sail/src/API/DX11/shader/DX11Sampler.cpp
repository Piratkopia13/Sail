#include "pch.h"
#include "DX11Sampler.h"
#include "Sail/Application.h"
#include "../DX11API.h"

namespace ShaderComponent {

	Sampler* Sampler::Create(Texture::ADDRESS_MODE adressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		return new DX11Sampler(adressMode, filter, bindShader, slot);
	}

	DX11Sampler::DX11Sampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot)
		: Sampler(addressMode, filter, bindShader, slot)
		, m_bindShader(bindShader)
		, m_slot(slot)
	{

		// Convert address mode to dx11 specific
		D3D11_TEXTURE_ADDRESS_MODE dx11AddressMode;
		switch (addressMode) {
		case Texture::WRAP:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case Texture::MIRROR:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		case Texture::CLAMP:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case Texture::BORDER:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		case Texture::MIRROR_ONCE:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			break;
		default:
			dx11AddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
		}

		// Convert filter to dx11 specific
		D3D11_FILTER dx11Filter;
		switch (filter) {
		case Texture::MIN_MAG_MIP_POINT:
			dx11Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case Texture::MIN_MAG_POINT_MIP_LINEAR:
			dx11Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case Texture::MIN_POINT_MAG_LINEAR_MIP_POINT:
			dx11Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case Texture::MIN_POINT_MAG_MIP_LINEAR:
			dx11Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case Texture::MIN_LINEAR_MAG_MIP_POINT:
			dx11Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case Texture::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			dx11Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case Texture::MIN_MAG_LINEAR_MIP_POINT:
			dx11Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case Texture::MIN_MAG_MIP_LINEAR:
			dx11Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case Texture::ANISOTROPIC:
			dx11Filter = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			dx11Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}


		// Set up sampler
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AddressU = dx11AddressMode;
		desc.AddressV = dx11AddressMode;
		desc.AddressW = dx11AddressMode;
		desc.Filter = dx11Filter;
		desc.MipLODBias = 0.f;
		desc.MaxAnisotropy = 0;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateSamplerState(&desc, &m_samplerState);

	}

	DX11Sampler::~DX11Sampler() {
		Memory::safeRelease(m_samplerState);
	}

	void DX11Sampler::bind() {
		if (m_bindShader & ShaderComponent::VS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->VSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::HS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->HSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::DS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->DSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::PS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->PSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::GS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->GSSetSamplers(m_slot, 1, &m_samplerState);
		if (m_bindShader & ShaderComponent::CS)
			Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->CSSetSamplers(m_slot, 1, &m_samplerState);
	}

}