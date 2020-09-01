#pragma once

#include "Sail/api/shader/Sampler.h"

namespace ShaderComponent {

	class VkSampler : public Sampler {
	public:
		VkSampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot);
		~VkSampler();

		virtual void bind() override;

	};

}