#include "pch.h"
#include "SVkSampler.h"
#include "../SVkUtils.h"

namespace ShaderComponent {

	Sampler* Sampler::Create(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		return SAIL_NEW SVkSampler(addressMode, filter, bindShader, slot);
	}

	SVkSampler::~SVkSampler() {
		vkDestroySampler(m_context->getDevice(), m_textureSampler, nullptr);
	}

	void SVkSampler::bind() {
		// Already bound via descriptor set created in SVkShader
	}

	const VkSampler& SVkSampler::get() const {
		return m_textureSampler;
	}

	SVkSampler::SVkSampler(Texture::ADDRESS_MODE addressMode, Texture::FILTER filter, BIND_SHADER bindShader, unsigned int slot) {
		m_context = Application::getInstance()->getAPI<SVkAPI>();

		// Convert address mode to vk specific
		VkSamplerAddressMode vkAddressMode;
		switch (addressMode) {
		case Texture::WRAP:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case Texture::MIRROR:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case Texture::CLAMP:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		case Texture::BORDER:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		case Texture::MIRROR_ONCE:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			break;
		default:
			vkAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		// Convert filter to vk specific
		VkFilter vkFilter;
		switch (filter) {
		case Texture::POINT:
			vkFilter = VK_FILTER_NEAREST;
			break;
		case Texture::LINEAR:
			vkFilter = VK_FILTER_LINEAR;
			break;
		case Texture::ANISOTROPIC:
			vkFilter = VK_FILTER_LINEAR;
			break;
		default:
			vkFilter = VK_FILTER_LINEAR;
		}


		// Set up sampler
		bool enableAnisotropic = (filter == Texture::ANISOTROPIC);
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = vkFilter;
		info.minFilter = vkFilter;
		info.addressModeU = vkAddressMode;
		info.addressModeV = vkAddressMode;
		info.addressModeW = vkAddressMode;
		info.anisotropyEnable = (enableAnisotropic) ? VK_TRUE : VK_FALSE;
		info.maxAnisotropy = 16.0f;
		info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		info.unnormalizedCoordinates = VK_FALSE;
		info.compareEnable = VK_FALSE;
		info.compareOp = VK_COMPARE_OP_ALWAYS;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.mipLodBias = 0.0f;
		info.minLod = 0.0f;
		info.maxLod = 0.0f;

		VK_CHECK_RESULT(vkCreateSampler(m_context->getDevice(), &info, nullptr, &m_textureSampler));
	}

}