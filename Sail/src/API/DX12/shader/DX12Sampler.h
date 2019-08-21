#pragma once

#include "Sail/api/shader/Sampler.h"

namespace ShaderComponent {

	class DX12Sampler : public Sampler {
	public:
		DX12Sampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot);
		~DX12Sampler();

		virtual void bind() override;

	};

}