#pragma once

#include "Sail/api/shader/Sampler.h"
#include "vulkan/vulkan_core.h"
#include "../SVkAPI.h"

namespace ShaderComponent {

	class SVkSampler : public Sampler {
	public:
		SVkSampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot);
		~SVkSampler();

		virtual void bind() override;

		const VkSampler& get() const;

	private:
		SVkAPI* m_context;
		VkSampler m_textureSampler;
	};
}