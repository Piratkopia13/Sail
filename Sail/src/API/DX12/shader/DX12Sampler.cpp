#include "pch.h"
#include "DX12Sampler.h"

namespace ShaderComponent {

	Sampler* Sampler::Create(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		return new DX12Sampler(addressMode, filter, bindShader, slot);
	}

	DX12Sampler::~DX12Sampler() {

	}

	void DX12Sampler::bind() {
		throw std::logic_error("The method or operation is not implemented.");
	}

	DX12Sampler::DX12Sampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {

	}

}