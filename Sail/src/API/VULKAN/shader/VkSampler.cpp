#include "pch.h"
#include "VkSampler.h"

namespace ShaderComponent {

	Sampler* Sampler::Create(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		return SAIL_NEW VkSampler(addressMode, filter, bindShader, slot);
	}

	VkSampler::~VkSampler() {

	}

	void VkSampler::bind() {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	VkSampler::VkSampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {

	}

}