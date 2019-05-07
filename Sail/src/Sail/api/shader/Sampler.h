#pragma once

#include "Sail/graphics/shader/component/BindShader.h"
#include "Sail/api/Texture.h"

namespace ShaderComponent {

	class Sampler {

	public:
		static Sampler* Sampler::Create(Texture::ADDRESS_MODE adressMode = Texture::WRAP, Texture::FILTER filter = Texture::MIN_MAG_MIP_LINEAR, BIND_SHADER bindShader = PS, UINT slot = 0);
		Sampler(Texture::ADDRESS_MODE adressMode = Texture::WRAP, Texture::FILTER filter = Texture::MIN_MAG_MIP_LINEAR, BIND_SHADER bindShader = PS, UINT slot = 0) {}
		~Sampler() {}

		virtual void bind() = 0;
	};

}