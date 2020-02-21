#pragma once

#include "Sail/graphics/shader/BindShader.h"
#include "Sail/api/Texture.h"
#include <map>

namespace ShaderComponent {

	class Sampler {

	public:
		static Sampler* Sampler::Create(Texture::ADDRESS_MODE addressMode = Texture::WRAP, Texture::FILTER filter = Texture::LINEAR, BIND_SHADER bindShader = PS, unsigned int slot = 0);
		Sampler(Texture::ADDRESS_MODE adressMode = Texture::WRAP, Texture::FILTER filter = Texture::LINEAR, BIND_SHADER bindShader = PS, unsigned int slot = 0) {}
		virtual ~Sampler() {}

		virtual void bind() = 0;

		struct ShaderInfo {
			int slot = -1;
			Texture::FILTER filter = Texture::ANISOTROPIC;
			Texture::ADDRESS_MODE addressMode = Texture::WRAP;
		};
		static std::map<std::string, ShaderInfo> GetShaderSlotsMap();

	private:
		static const std::map<std::string, ShaderInfo> s_shaderSlots;
	};

}