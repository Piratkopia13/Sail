#pragma once

#include <d3d11.h>
#include "Sail/api/shader/Sampler.h"

namespace ShaderComponent {

	class DX11Sampler : public Sampler {

	public:
		DX11Sampler(Texture::ADDRESS_MODE addressMode = Texture::WRAP, Texture::FILTER filter = Texture::LINEAR, BIND_SHADER bindShader = PS, unsigned int slot = 0);
		~DX11Sampler();

		virtual void bind() override;
	
	private:
		ID3D11SamplerState* m_samplerState;
		BIND_SHADER m_bindShader;
		unsigned int m_slot;
	};

}