#pragma once

#include <d3d11.h>
#include "BindShader.h"

namespace ShaderComponent {

	class Sampler {

	public:
		Sampler(Texture::ADDRESS_MODE adressMode = Texture::WRAP, Texture::FILTER filter = Texture::MIN_MAG_MIP_LINEAR, BIND_SHADER bindShader = PS, UINT slot = 0);
		~Sampler();

		void bind();

	private:
		ID3D11SamplerState* m_samplerState;
		BIND_SHADER m_bindShader;
		UINT m_slot;

	};

}